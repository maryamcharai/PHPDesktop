# ifndef MONGOOSE_HEADER_INCLUDED
# define   MONGOOSE_HEADER_INCLUDED

# include  < stdio.h >
# include  < stddef.h >

# ifdef __cplusplus
extern  " C " {
#endif // __cplusplus

struct mg_context;     // Handle pour le service HTTP lui-même
struct mg_connection;  // Handle pour la connexion individuelle


// Cette structure contient des informations sur la requête HTTP.
struct mg_request_info {
  const  char * request_method; // "GET", "POST", etc.
  const  char * uri;            // URI décodé par URL
  const  char * http_version;   // Par exemple "1.0", "1.1"
  const  char * query_string;   // partie URL après '?', Sans '?' Ou NULL
  const  char * remote_user;    // utilisateur authentifié ou NULL si aucune autorisation n'est utilisée
  long remote_ip;             // adresse IP du client
  int remote_port;            // port du client
  int is_ssl;                 // 1 si SSL-ed, 0 sinon
  void * user_data;            // Pointeur de données utilisateur passé à mg_start ()
  void * conn_data;            // données utilisateur spécifiques à la connexion

  int num_headers;            // Nombre d'en-têtes HTTP
  struct mg_header {
    const  char * name;         // nom de l'en-tête HTTP
    const  char * value;        // valeur d'en-tête HTTP
  } http_headers [ 64 ];         // 64 en-têtes maximum
};


// Cette structure doit être passée à mg_start (), pour que les mangoustes sachent
// les callbacks à invoquer. Pour une description détaillée, voir
// https://github.com/valenok/mongoose/blob/master/UserManual.md
struct mg_callbacks {
  // Appelé lorsque mongoose a reçu une nouvelle requête HTTP.
  // Si le rappel retourne une valeur autre que zéro,
  // callback doit traiter la demande en envoyant des en-têtes et un corps HTTP valides,
  // et mangouste ne fera aucun traitement ultérieur.
  // Si callback renvoie 0, Mongoose traite la demande elle-même. Dans ce cas,
  // callback ne doit envoyer aucune donnée au client.
  int   (* begin_request) ( struct mg_connection *);

  // Appelé lorsque Mangouste a fini de traiter la demande.
  void (* end_request) ( const  struct mg_connection *, int reply_status_code);

  // Appelé lorsque mangouste est sur le point de consigner un message. Si le rappel revient
  // non nul, mangouste n'enregistre rien.
  int   (* log_message) ( const  struct mg_connection *, const  char * message);

  // Appelé lorsque mongoose initialise la bibliothèque SSL.
  int   (* init_ssl) ( void * ssl_context, void * user_data);

  // Appelé lorsque la demande websocket est reçue, avant la prise de contact websocket.
  // Si callback renvoie 0, la mangouste procède à une poignée de main, sinon
  // la cinnection est fermée immédiatement.
  int (* websocket_connect) ( const  struct mg_connection *);

  // Appelé lorsque Websocket Handshake est terminé avec succès, et
  // la connexion est prête pour l'échange de données.
  void (* websocket_ready) ( struct mg_connection *);

  // Appelé lorsque la trame de données a été reçue du client.
  // Paramètres:
  //     bits: premier octet du cadre websocket, voir RFC websocket à l'adresse
  //           http://tools.ietf.org/html/rfc6455, section 5.2
  //     data, data_len: payload, avec masque (le cas échéant) déjà appliqué.
  // valeur de retour:
  //     non-0: garder cette connexion websocket ouverte.
  //     0: ferme cette connexion websocket.
  int   (* websocket_data) ( struct mg_connection *, int bits,
                         char * data, size_t data_len);

  // Appelé quand Mangouste tente d'ouvrir un fichier. Utilisé pour intercepter un fichier ouvert
  // appelle et sert les données de fichier de la mémoire à la place.
  // Paramètres:
  //     chemin: chemin complet du fichier à ouvrir.
  //     data_len: Placeholder pour la taille du fichier, si le fichier est servi à partir de la mémoire.
  // valeur de retour:
  //     NULL: ne sert pas le fichier de la mémoire, continue avec l'ouverture du fichier normale.
  //     non-NULL: pointeur sur le contenu du fichier en mémoire. data_len doit être
  //               initié avec la taille du bloc de mémoire.
  const  char * (* open_file) ( const  struct mg_connection *,
                             const  char * path, size_t * data_len);

  // Appelé quand mongoose est sur le point de servir la page du serveur Lua (fichier .lp), si
  // Le support Lua est activé.
  // Paramètres:
  //    lua_context: "pointeur de lua_State *".
  void (* init_lua) ( struct mg_connection *, void * lua_context);

  // Appelé quand mongoose a téléchargé un fichier dans un répertoire temporaire en tant que
  // résultat de l'appel de mg_upload ().
  // Paramètres:
  //     fichier_fichier: chemin d'accès complet au fichier téléchargé.
  void (* upload) ( struct mg_connection *, const  char * nom_fichier);

  // Appelé lorsque mongoose est sur le point d'envoyer une erreur HTTP au client.
  // L' implémentation de ce rappel permet de créer des pages d'erreur personnalisées.
  // Paramètres:
  //    status: code d'état d'erreur HTTP.
  int   (* http_error) ( struct mg_connection *, int status);
};

// Démarrer le serveur Web.
//
// Paramètres:
//    callbacks: structure mg_callbacks avec des rappels définis par l'utilisateur.
//    options: liste terminée par NULL des paires nom_option, valeur_option qui
//             spécifie les paramètres de configuration Mongoose.
//
// Effets secondaires: sous UNIX, ignore les signaux SIGCHLD et SIGPIPE. Si coutume
// le     traitement est requis pour ceux-ci, les gestionnaires de signaux doivent être configurés
//     après avoir appelé mg_start ().
//
//
// Exemple:
//    const char * options [] = {
//      "racine_document", "/ var / www",
//      "listening_ports", "80,443s",
//      NULL
//    };
//    struct mg_context * ctx = mg_start (& my_func, NULL, options);
//
// Voir https://github.com/valenok/mongoose/blob/master/UserManual.md
// pour la liste des options valides et leurs valeurs possibles.
//
// retour:
//    contexte du serveur Web ou NULL en cas d'erreur.
struct mg_context * mg_start ( const  struct mg_callbacks *, callbacks
                            void * user_data,
                            const  char ** configuration_options);


// Arrête le serveur Web.
//
// Doit être appelé en dernier, lorsqu'une application veut arrêter le serveur Web et
// libère toutes les ressources associées. Cette fonction bloque jusqu’à ce que tout Mongoose
// les threads sont arrêtés. Le pointeur de contexte devient invalide.
void  mg_stop ( struct mg_context *);


// Récupère la valeur d'un paramètre de configuration particulier.
// La valeur renvoyée est en lecture seule. Mongoose ne permet pas de changer
// configuration au moment de l'exécution.
// Si le nom du paramètre donné n'est pas valide, NULL est renvoyé. Pour valide
// noms, la valeur renvoyée est garantie d'être non-NULL. Si le paramètre n'est pas
// set, une chaîne de longueur nulle est renvoyée.
const  carbonisation * mg_get_option ( const  struct mg_context * ctx, const  carbonisation nom *);


// Retourne un tableau de chaînes qui représentent des options de configuration valides.
// Pour chaque option, le nom de l'option et la valeur par défaut sont renvoyés, c'est-à-dire le
// nombre d'entrées dans le tableau est égal à number_of_options x 2.
// Le tableau est terminé par NULL.
const  char ** mg_get_valid_option_names ( void );


// Ajoute, modifie ou supprime l'entrée dans le fichier de mots de passe.
//
// Cette fonction permet à une application de manipuler des fichiers .htpasswd sur le
// Fly en ajoutant, en supprimant et en modifiant les enregistrements d'utilisateurs. C'est l'un des
// plusieurs façons d'implémenter l'authentification côté serveur. Pour un autre,
// méthode basée sur les cookies, veuillez vous référer aux exemples / chat.c dans l'arborescence des sources.
//
// Si le mot de passe n'est pas NULL, l'entrée est ajoutée (ou modifiée si elle existe déjà).
// Si le mot de passe est NULL, l'entrée est supprimée.
//
// retour:
//    1 en cas de succès, 0 en cas d'erreur.
int  mg_modify_passwords_file ( const  char * nom_fichier_passwords ,
                             const  char * domain,
                             const  char * user,
                             const  char * mot de passe);


// Retourne les informations associées à la demande.
struct mg_request_info * mg_get_request_info ( struct mg_connection *);


// Envoie des données au client.
// retour:
//   0 lorsque la connexion a été fermée
//   -1 en cas d'erreur
//   > 0 nombre d'octets écrits en cas de succès
int  mg_write ( struct mg_connection *, const  void * buf, size_t len);


// Envoi de données à un client Websocket encapsulé dans un cadre websocket.
// Il est dangereux de lire / écrire sur cette connexion depuis un autre thread.
// Cette fonction est disponible lorsque mongoose est compilé avec -DUSE_WEBSOCKET
//
// retour:
//   0 lorsque la connexion a été fermée
//   -1 en cas d'erreur
//   > 0 nombre d'octets écrits en cas de succès
int  mg_websocket_write ( struct mg_connection * conn, opcode int ,
                       const  char * data, size_t data_len);

// Opcodes, de http://tools.ietf.org/html/rfc6455
enum {
  WEBSOCKET_OPCODE_CONTINUATION = 0x0 ,
  WEBSOCKET_OPCODE_TEXT = 0x1 ,
  WEBSOCKET_OPCODE_BINARY = 0x2 ,
  WEBSOCKET_OPCODE_CONNECTION_CLOSE = 0x8 ,
  WEBSOCKET_OPCODE_PING = 0x9 ,
  WEBSOCKET_OPCODE_PONG = 0xa
};


// Macros permettant d'activer les vérifications spécifiques au compilateur pour les arguments de type printf.
# undef PRINTF_FORMAT_STRING
# si défini (_MSC_VER) && _MSC_VER> = 1400
# include  < sal.h >
# si défini (_MSC_VER) && _MSC_VER> 1400
# define  PRINTF_FORMAT_STRING ( s ) _Printf_format_string_ s
# else
# define  PRINTF_FORMAT_STRING ( s ) __format_string s
# endif
# else
# define  PRINTF_FORMAT_STRING ( s )
# endif

# ifdef __GNUC__
# define  PRINTF_ARGS ( x, y ) __attribute __ ((format (printf, x, y)))
# else
# define  PRINTF_ARGS ( x, y )
# endif

// Envoie des données au client en utilisant la sémantique printf ().
//
// Fonctionne exactement comme mg_write (), mais permet de formater un message.
int  mg_printf ( struct mg_connection *,
              PRINTF_FORMAT_STRING ( const  char * fmt), ...) PRINTF_ARGS ( 2 , 3 );


// Envoie le contenu du fichier entier avec les en-têtes HTTP.
void  mg_send_file ( struct mg_connection * conn, const  char * path);


// Lecture des données depuis l'extrémité distante, retourne le nombre d'octets lus.
// retour:
//    0 connexion a été fermée par un homologue. Aucune autre donnée ne peut être lue.
//    <0 erreur de lecture. Aucune autre donnée n'a pu être lue à partir de la connexion.
//    > 0 nombre d'octets lus dans le tampon.
int  mg_read ( struct mg_connection *, void * buf, size_t len);


// Récupère la valeur d'un en-tête HTTP particulier.
//
// Ceci est une fonction d'assistance. Il traverse request_info-> http_headers array,
// et si l'en-tête est présent dans le tableau, retourne sa valeur. Si c'est
// non présent, NULL est renvoyé.
const  char * mg_get_header ( const  struct mg_connection *, const  char * nom);


// Obtient une valeur de variable de formulaire particulière.
//
// Paramètres:
//    data: pointeur sur le tampon form-uri. Cela pourrait être soit des données POST,
//          ou request_info.query_string.
//    data_len: longueur des données encodées.
//    nom_var: nom de variable à décoder du tampon
//    dst: tampon de destination pour la variable décodée
//    dst_len: longueur du tampon de destination
//
// retour:
//    En cas de succès, longueur de la variable décodée.
//    En cas d'erreur:
//       -1 (variable non trouvée).
//       -2 (le tampon de destination est NULL, de longueur zéro ou trop petit pour contenir le
//           variable décodée).
//
// Le tampon de destination a la garantie d'être '\ 0' - terminé s'il ne l'est pas
// NULL ou zéro longueur.
int  mg_get_var ( const  char * data, size_t data_len,
               const  char * nom_var, char * dst, size_t dst_len);

// Récupère la valeur de certaines variables de cookie dans le tampon de destination.
//
// Il est garanti que le tampon de destination se termine par "\ 0". En cas de
// échec, dst [0] == '\ 0'. Notez que RFC autorise plusieurs occurrences de la même
// paramètre. Cette fonction ne renvoie que la première occurrence.
//
// retour:
//    En cas de succès, indiquez la longueur.
//    En cas d'erreur:
//       -1 (soit l'en-tête "Cookie:" n'est pas présent, soit l'en-tête demandé
//           paramètre non trouvé).
//       -2 (le tampon de destination est NULL, de longueur zéro ou trop petit pour contenir le
//           valeur).
int  mg_get_cookie ( const  char * cookie, const  char * nom_var,
                  char * buf, size_t buf_len);


// Télécharger les données du serveur Web distant.
//    hôte: nom de l'hôte auquel se connecter, par exemple "foo.com" ou "10.12.40.1".
//    port: numéro de port, par exemple 80.
//    use_ssl: utilise la connexion SSL.
//    error_buffer, error_buffer_size: espace réservé pour le message d'erreur.
//    request_fmt, ...: requête HTTP.
// retour:
//    En cas de succès, pointeur valide sur la nouvelle connexion, adapté à mg_read ().
//    En cas d'erreur, NULL. error_buffer contient un message d'erreur.
// Exemple:
//    char ebuf [100];
//    struct mg_connection * conn;
//    conn = mg_download ("google.com", 80, 0, ebuf, sizeof (ebuf),
//                       "% s", "GET / HTTP / 1.0 \ r \ nHôte: google.com \ r \ n \ r \ n");
struct mg_connection * mg_download ( const  ombles * host, int port, int use_ssl,
                                  char * error_buffer, size_t error_buffer_size,
                                  PRINTF_FORMAT_STRING ( const  char * request_fmt),
                                  ...) PRINTF_ARGS ( 6 , 7 );


// Ferme la connexion ouverte par mg_download ().
void  mg_close_connection ( struct mg_connection * conn);


// fonctionnalité de téléchargement de fichier. Chaque fichier téléchargé est enregistré dans un fichier temporaire.
// le fichier et l'événement MG_UPLOAD sont envoyés.
// Renvoie le nombre de fichiers téléchargés.
int  mg_upload ( struct mg_connection * conn, const  char * destination_dir);


// Fonction pratique - crée un fil détaché.
// Retour: 0 en cas de succès, non-0 en cas d'erreur.
typedef  void * (* mg_thread_func_t ) ( void *);
int  mg_start_thread ( mg_thread_func_t f, void * p);


// Retourne le type mime intégré pour le nom de fichier donné.
// Pour les extensions non reconnues, "text / plain" est renvoyé.
const  char * mg_get_builtin_mime_type ( const  char * nom_fichier);


// Retourne la version de Mongoose.
const  char * mg_version ( void );

// tampon de décodage d'URL dans le tampon de destination.
// 0-termine le tampon de destination.
// les données codées en URL de formulaire diffèrent du codage URI en ce sens
// utilise '+' comme caractère pour l'espace, voir RFC 1866 section 8.2.1
// http://ftp.ics.uci.edu/pub/ietf/html/rfc1866.txt
// Return: longueur des données décodées, ou -1 si le tampon dst est trop petit.
int  mg_url_decode ( const  char * src, int src_len, char * dst,
                  int dst_len, int is_form_url_encoded);

// MD5 hash donné des chaînes.
// Le tampon 'buf' doit avoir une longueur de 33 octets. Varargs est une liste terminée par NULL
// Chaînes ASCIIz. Lorsque la fonction revient, buf contiendra des informations lisibles par l'homme
// hash MD5. Exemple:
//    char buf [33];
//    mg_md5 (buf, "aa", "bb", NULL);
char * mg_md5 ( char buf [ 33 ], ...);


# ifdef __cplusplus
}
# endif  // __cplusplus

#endif // MONGOOSE_HEADER_INCLUDED
