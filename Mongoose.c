# si défini (_WIN32)
# if ! defini (_CRT_SECURE_NO_WARNINGS)
# define  _CRT_SECURE_NO_WARNINGS  // Désactiver l'avertissement de désapprobation dans VS2005
# endif
# else
# ifdef __linux__
# define  _XOPEN_SOURCE  600      // Pour flockfile () sous Linux
# endif
# define  _LARGEFILE_SOURCE      // Activer les décalages de fichiers 64 bits
# define  __STDC_FORMAT_MACROS   // <inttypes.h> veut cela pour C ++
# define  __STDC_LIMIT_MACROS    // C ++ veut cela pour INT64_MAX
# endif

# si défini (_MSC_VER)
// l' expression conditionnelle est constante: introduite par FD_SET (..)
# pragma warning (disable: 4127)
// initialiseur d'agrégat non constant: émis en raison d'un manque de prise en charge de C99
# pragma warning (disable: 4204)
# endif

// Désactive WIN32_LEAN_AND_MEAN.
// Cela fait que windows.h inclut toujours winsock2.h
# ifdef WIN32_LEAN_AND_MEAN
# undef WIN32_LEAN_AND_MEAN
# endif

# si défini (__ SYMBIAN32__)
# define  NO_SSL  // SSL n'est pas supporté
# define  NO_CGI  // CGI n'est pas supporté
# define  PATH_MAX FILENAME_MAX
# endif  // __SYMBIAN32__

# ifndef _WIN32_WCE // Certaines informations # ANSI ne sont pas disponibles sous Windows CE
# include  < sys / types.h >
# include  < sys / stat.h >
# include  < errno.h >
# include  < signal.h >
# include  < fcntl.h >
# endif  // ! _WIN32_WCE

# include  < time.h >
# include  < stdlib.h >
# include  < stdarg.h >
# include  < assert.h >
# include  < string.h >
# include  < ctype.h >
# include  < limits.h >
# include  < stddef.h >
# include  < stdio.h >

# si défini (_WIN32) &&! defini (__ SYMBIAN32__) // spécifique à Windows
# undef _WIN32_WINNT
# define  _WIN32_WINNT  0x0400  

# endif  // ! NO_SSL

  // Deallocate context lui-même
  libre (ctx);
}

void  mg_stop ( struct mg_context * ctx) {
  ctx-> stop_flag = 1 ;

  // Attend que mg_fini () s'arrête
  while (ctx-> stop_flag ! = 2 ) {
    ( vide ) mg_sleep ( 10 );
  }
  free_context (ctx);

# si défini (_WIN32) &&! defini (__ SYMBIAN32__)
  ( void ) WSACleanup ();
# endif  // _WIN32
}

struct mg_context * mg_start ( const  struct mg_callbacks *, callbacks
                            void * user_data,
                            const  char ** options) {
  struct mg_context * ctx;
  const  char * nom, * valeur, * valeur_défaut;
  int i;

# si défini (_WIN32) &&! defini (__ SYMBIAN32__)
  Données WSADATA;
  WSAStartup ( MAKEWORD ( 2 , 2 ), & data);
  InitializeCriticalSection (& global_log_file_lock);
# endif  // _WIN32

  // Alloue le contexte et initialise les valeurs par défaut des cas généraux raisonnables.
  // TODO (lsm): traite correctement les erreurs ici.
  if ((ctx = ( struct mg_context *) calloc ( 1 , sizeof (* ctx))) == NULL ) {
    return  NULL ;
  }
  ctx-> callbacks = * callbacks;
  ctx-> user_data = user_data;

  while (options && (name = * options ++)! = NULL ) {
    if ((i = get_option_index (name)) == - 1 ) {
      cry ( fc (ctx), " option non valide: % s " , nom);
      free_context (ctx);
      return  NULL ;
    } else  if ((value = * options ++) == NULL ) {
      cry ( fc (ctx), " % s : la valeur de l'option ne peut pas être NULL " , nom);
      free_context (ctx);
      return  NULL ;
    }
    if (ctx-> config [i]! = NULL ) {
      cry ( fc (ctx), " avertissement: % s : option de duplication " , nom);
      gratuit (ctx-> config [i]);
    }
    ctx-> config [i] = mg_strdup (valeur);
    DEBUG_TRACE (( " [ % s ] -> [ % s ] " , nom, valeur));
  }

  // Définir la valeur par défaut si nécessaire
  pour (i = 0 ; config_options [i * 2 ]! = NULL ; i ++) {
    default_value = config_options [i * 2 + 1 ];
    if (ctx-> config [i] == NULL && default_value! = NULL ) {
      ctx-> config [i] = mg_strdup (default_value);
    }
  }

  // NOTE (lsm): l'ordre est important ici. Les certificats SSL doivent
  // être initialisé avant d'écouter les ports. UID doit être défini en dernier.
  si (! set_gpass_option (ctx) ||
# si ! defini (NO_SSL)
      ! set_ssl_option (ctx) ||
#endif
      ! set_ports_option (ctx) ||
# si ! defini (_WIN32)
      ! set_uid_option (ctx) ||
#endif
      ! set_acl_option (ctx)) {
    free_context (ctx);
    return  NULL ;
  }

# if ! defini (_WIN32) &&! defin (__ SYMBIAN32__)
  // Ignorer le signal SIGPIPE, donc si le navigateur annule la demande, il
  // ne tue pas tout le processus.
  ( void ) signal (SIGPIPE, SIG_IGN);
  // Ignore également SIGCHLD pour permettre au système d'exploitation de récupérer les zombies correctement.
  ( void ) signal (SIGCHLD, SIG_IGN);
# endif  // ! _WIN32

  ( void ) pthread_mutex_init (& ctx-> mutex , NULL );
  ( void ) pthread_cond_init (& ctx-> cond , NULL );
  ( void ) pthread_cond_init (& ctx-> sq_empty , NULL );
  ( void ) pthread_cond_init (& ctx-> sq_full , NULL );

  // Démarrer le fil maître (en écoute)
  mg_start_thread (master_thread, ctx);

  // Démarrer les threads de travail
  pour (i = 0 ; i < atoi (ctx-> config [NUM_THREADS]); i ++) {
    if ( mg_start_thread (worker_thread, ctx)! = 0 ) {
      cry ( fc (ctx), " Impossible de démarrer le thread de travail: % ld " , ( long ) ERRNO);
    } else {
      ctx-> num_threads ++;
    }
  }

  revenir ctx;
}
