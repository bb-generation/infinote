/* libinfinity - a GObject-based infinote implementation
 * Copyright (C) 2007-2011 Armin Burgmeier <armin@arbur.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <infinoted/infinoted-startup.h>
#include <infinoted/infinoted-util.h>
#include <infinoted/infinoted-creds.h>

#include <infinoted/infinoted-pam.h>

#include <libinfinity/common/inf-cert-util.h>
#include <libinfinity/common/inf-init.h>
#include <libinfinity/common/inf-error.h>
#include <libinfinity/inf-i18n.h>
#include <libinfinity/inf-config.h>

#include <gnutls/x509.h>

#include <string.h>
#include <stdlib.h>

static void
infinoted_startup_free_certificate_array(gnutls_x509_crt_t* certificates,
                                         guint n_certificates)
{
  guint i;
  for(i = 0; i < n_certificates; ++ i)
    gnutls_x509_crt_deinit(certificates[i]);
  g_free(certificates);
}

static gnutls_x509_privkey_t
infinoted_startup_load_key(gboolean create_key,
                           const gchar* key_file,
                           GError** error)
{
  gnutls_x509_privkey_t key;

  if(create_key == TRUE)
  {
    if(infinoted_util_create_dirname(key_file, error) == FALSE)
      return NULL;

    /* TODO: Open the key file beforehand */

    infinoted_util_log_info(_("Generating 2048 bit RSA private key..."));
    key = infinoted_creds_create_key(error);

    if(key == NULL)
      return NULL;

    if(infinoted_creds_write_key(key, key_file, error) == FALSE)
    {
      gnutls_x509_privkey_deinit(key);
      return NULL;
    }
  }
  else
  {
    key = infinoted_creds_read_key(key_file, error);
  }

  return key;
}

static gnutls_x509_crt_t*
infinoted_startup_load_certificate(gboolean create_self_signed_certificate,
                                   gnutls_x509_privkey_t key,
                                   const gchar* certificate_file,
                                   const gchar* certificate_chain_file,
                                   guint* n_certificates,
                                   GError** error)
{
  gnutls_x509_crt_t* result;
  gnutls_x509_crt_t cert;
  GPtrArray* certs;
  GPtrArray* chain_certs;

  if(create_self_signed_certificate == TRUE)
  {
    if(infinoted_util_create_dirname(certificate_file, error) == FALSE)
      return NULL;

    infinoted_util_log_info(_("Generating self-signed certificate..."));
    cert = infinoted_creds_create_self_signed_certificate(key, error);
    if(cert == NULL) return NULL;

    if(inf_cert_util_save_file(&cert, 1, certificate_file, error) == FALSE)
    {
      gnutls_x509_crt_deinit(cert);
      return NULL;
    }
    else
    {
      result = g_malloc(sizeof(gnutls_x509_crt_t));
      *result = cert;
      *n_certificates = 1;
    }
  }
  else
  {
    certs = inf_cert_util_load_file(certificate_file, NULL, error);
    if(certs == NULL) return NULL;

    if(certificate_chain_file != NULL)
    {
      chain_certs =
        inf_cert_util_load_file(certificate_chain_file, certs, error);

      if(chain_certs == NULL)
      {
        result = (gnutls_x509_crt_t*)g_ptr_array_free(certs, FALSE);
        infinoted_startup_free_certificate_array(result, *n_certificates);
        return NULL;
      }
    }

    *n_certificates = certs->len;
    result = (gnutls_x509_crt_t*)g_ptr_array_free(certs, FALSE);
  }

  return result;
}

static gboolean
infinoted_startup_load_credentials(InfinotedStartup* startup,
                                   GError** error)
{
  if(startup->options->security_policy !=
     INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED)
  {
    startup->private_key = infinoted_startup_load_key(
      startup->options->create_key,
      startup->options->key_file,
      error
    );

    if(startup->private_key == NULL)
      return FALSE;

    startup->certificates = infinoted_startup_load_certificate(
      startup->options->create_certificate,
      startup->private_key,
      startup->options->certificate_file,
      startup->options->certificate_chain_file,
      &startup->n_certificates,
      error
    );

    if(startup->certificates == NULL)
      return FALSE;

    startup->credentials = infinoted_creds_create_credentials(
      startup->private_key,
      startup->certificates,
      startup->n_certificates,
      error
    );

    if(startup->credentials == NULL)
      return FALSE;
  }

  return TRUE;
}

static gboolean
infinoted_startup_load_options(InfinotedStartup* startup,
                               int* argc,
                               char*** argv,
                               GError** error)
{
  const gchar* const* system_config_dirs;
  guint n_system_config_dirs;
  const gchar* user_config_dir;

  const gchar* const* dir;
  guint i;

  gchar** config_files;

  system_config_dirs = g_get_system_config_dirs();
  user_config_dir = g_get_user_config_dir();

  n_system_config_dirs = 0;
  if(system_config_dirs != NULL)
  {
    for(dir = system_config_dirs; *dir != NULL; ++ dir)
      ++ n_system_config_dirs;
  }

  config_files = g_malloc( (n_system_config_dirs + 2) * sizeof(gchar*));
  config_files[n_system_config_dirs + 1] = NULL;
  config_files[n_system_config_dirs] =
    g_build_filename(user_config_dir, "infinoted.conf", NULL);

  for(i = 0; i < n_system_config_dirs; ++ i)
  {
    config_files[n_system_config_dirs - i - 1] =
      g_build_filename(system_config_dirs[i], "infinoted.conf", NULL);
  }

  startup->options =
    infinoted_options_new(
      (gchar const* const*) config_files, argc, argv, error);

  for(i = 0; i < n_system_config_dirs + 1; ++ i)
    g_free(config_files[i]);
  g_free(config_files);

  if(startup->options == NULL)
    return FALSE;

  return TRUE;
}

static void
infinoted_startup_sasl_callback_set_error(InfXmppConnection* connection,
                                          InfAuthenticationDetailError code,
                                          const GError* error)
{
  GError* own_error;
  own_error = NULL;

  if(!error)
  {
    own_error = g_error_new_literal(
      inf_authentication_detail_error_quark(),
      code,
      inf_authentication_detail_strerror(code)
    );

    inf_xmpp_connection_set_sasl_error(connection, own_error);
    g_error_free(own_error);
  }
  else
  {
    inf_xmpp_connection_set_sasl_error(connection, error);
  }
}

static void
infinoted_startup_sasl_callback(InfSaslContextSession* session,
                                Gsasl_property prop,
                                gpointer session_data,
                                gpointer user_data)
{
  InfinotedStartup* startup;
  const char* password;
  InfXmppConnection* xmpp;

#ifdef LIBINFINITY_HAVE_PAM
  const char* username;
  const gchar* pam_service;
  GError* error;
#endif

  xmpp = INF_XMPP_CONNECTION(session_data);

  switch(prop)
  {
  case GSASL_VALIDATE_SIMPLE:
    startup = (InfinotedStartup*)user_data;
    password = inf_sasl_context_session_get_property(session, GSASL_PASSWORD);
#ifdef LIBINFINITY_HAVE_PAM
    username = inf_sasl_context_session_get_property(session, GSASL_AUTHID);
    pam_service = startup->options->pam_service;
    if(pam_service != NULL)
    {
      error = NULL;
      if(!infinoted_pam_authenticate(pam_service, username, password))
      {
        infinoted_startup_sasl_callback_set_error(
          xmpp,
          INF_AUTHENTICATION_DETAIL_ERROR_AUTHENTICATION_FAILED,
          NULL
        );

        inf_sasl_context_session_continue(
          session,
          GSASL_AUTHENTICATION_ERROR
        );
      }
      else if(!infinoted_pam_user_is_allowed(startup->options,
                                             username,
                                             &error))
      {
        infinoted_startup_sasl_callback_set_error(
          xmpp,
          INF_AUTHENTICATION_DETAIL_ERROR_USER_NOT_AUTHORIZED,
          error
        );

        inf_sasl_context_session_continue(
          session,
          GSASL_AUTHENTICATION_ERROR
        );
      }
      else
      {
        inf_sasl_context_session_continue(session, GSASL_OK);
      }
    }
    else
#endif /* LIBINFINITY_HAVE_PAM */
    {
      g_assert(startup->options->password != NULL);
      if(strcmp(startup->options->password, password) == 0)
      {
        inf_sasl_context_session_continue(session, GSASL_OK);
      }
      else
      {
        infinoted_startup_sasl_callback_set_error(
          xmpp,
          INF_AUTHENTICATION_DETAIL_ERROR_AUTHENTICATION_FAILED,
          NULL
        );

        inf_sasl_context_session_continue(
          session,
          GSASL_AUTHENTICATION_ERROR
        );
      }
    }

    break;
  default:
    inf_sasl_context_session_continue(session, GSASL_AUTHENTICATION_ERROR);
    break;
  }
}

static gboolean
infinoted_startup_load(InfinotedStartup* startup,
                       int* argc,
                       char*** argv,
                       GError** error)
{
  gboolean requires_password;

  if(infinoted_startup_load_options(startup, argc, argv, error) == FALSE)
    return FALSE;

  if(infinoted_startup_load_credentials(startup, error) == FALSE)
    return FALSE;

  requires_password = startup->options->password != NULL;
#ifdef LIBINFINITY_HAVE_PAM
  requires_password =
    requires_password || startup->options->pam_service != NULL;
#endif /* LIBINFINITY_HAVE_PAM */

  if(requires_password)
  {
    startup->sasl_context = inf_sasl_context_new(error);
    if(!startup->sasl_context) return FALSE;

    inf_sasl_context_set_callback(
      startup->sasl_context,
      infinoted_startup_sasl_callback,
      startup
    );
  }

  return TRUE;
}

/**
 * infinoted_startup_new:
 * @error: Location to store error information, if any.
 *
 * Creates parameters for starting an infinote daemon. This involves option
 * parsing, reading config files, reading or creating data for TLS
 * (private key and certificate).
 *
 * Returns: A new #InfinotedStartup. Free with infinoted_startup_free().
 */
InfinotedStartup*
infinoted_startup_new(int* argc,
                      char*** argv,
                      GError** error)
{
  InfinotedStartup* startup;

  if(!inf_init(error))
    return NULL;

  startup = g_slice_new(InfinotedStartup);
  startup->options = NULL;
  startup->private_key = NULL;
  startup->certificates = NULL;
  startup->n_certificates = 0;
  startup->credentials = NULL;
  startup->sasl_context = NULL;

  if(infinoted_startup_load(startup, argc, argv, error) == FALSE)
  {
    infinoted_startup_free(startup);
    return NULL;
  }

  return startup;
}

/**
 * infinoted_startup_free:
 * @startup: A #InfinotedStartup.
 *
 * Frees all ressources allocated by @startup.
 */
void
infinoted_startup_free(InfinotedStartup* startup)
{
  if(startup->credentials != NULL)
    inf_certificate_credentials_unref(startup->credentials);

  if(startup->certificates != NULL)
  {
    infinoted_startup_free_certificate_array(
      startup->certificates,
      startup->n_certificates
    );
  }

  if(startup->private_key != NULL)
    gnutls_x509_privkey_deinit(startup->private_key);

  if(startup->options != NULL)
    infinoted_options_free(startup->options);

  if(startup->sasl_context != NULL)
    inf_sasl_context_unref(startup->sasl_context);

  g_slice_free(InfinotedStartup, startup);
  inf_deinit();
}

/* vim:set et sw=2 ts=2: */
