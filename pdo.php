<? php
// Fonctions d'assistance PDO.
// Copyright (c) 2012-2013, auteurs du bureau PHP. Tous les droits sont réservés.
// Licence: Nouvelle licence BSD.
// Site Web: http://code.google.com/p/phpdesktop/
fonction  PDO_Connect ( $ dsn , $ user = " " , $ password = " " )
{
    Global  AOP ;
    $ PDO  =  new  PDO ( $ dsn , $ utilisateur , $ mot de passe );
    $ PDO -> setAttribute ( PDO :: ATTR_ERRMODE , PDO :: ERRMODE_WARNING );
}
fonction  PDO_FetchOne ( $ query , $ params = null )
{
    Global  AOP ;
    if ( isset ( $ params )) {
        $ stmt  =  $ PDO -> prepare ( $ query );
        $ stmt -> execute ( $ params );
    } else {
        $ stmt  =  $ PDO -> query ( $ query );
    }
    $ row  =  $ stmt -> fetch ( PDO :: FETCH_NUM );
    if ( $ row ) {
        return  $ row [ 0 ];
    } else {
        retourne  faux ;
    }
}
fonction  PDO_FetchRow ( $ query , $ params = null )
{
    Global  AOP ;
    if ( isset ( $ params )) {
        $ stmt  =  $ PDO -> prepare ( $ query );
        $ stmt -> execute ( $ params );
    } else {
        $ stmt  =  $ PDO -> query ( $ query );
    }
    return  $ stmt -> fetch ( PDO :: FETCH_ASSOC );
}
fonction  PDO_FetchAll ( $ query , $ params = null )
{
    Global  AOP ;
    if ( isset ( $ params )) {
        $ stmt  =  $ PDO -> prepare ( $ query );
        $ stmt -> execute ( $ params );
    } else {
        $ stmt  =  $ PDO -> query ( $ query );
    }
    return  $ stmt -> fetchAll ( PDO :: FETCH_ASSOC );
}
fonction  PDO_FetchAssoc ( $ query , $ params = null )
{
    Global  AOP ;
    if ( isset ( $ params )) {
        $ stmt  =  $ PDO -> prepare ( $ query );
        $ stmt -> execute ( $ params );
    } else {
        $ stmt  =  $ PDO -> query ( $ query );
    }
    $ rows  =  $ stmt -> fetchAll ( PDO :: FETCH_NUM );
    $ assoc  =  array ();
    foreach ( $ rows en  tant que  $ row ) {
        $ assoc [ $ row [ 0 ]] =  $ row [ 1 ];
    }
    return  $ assoc ;
}
fonction  PDO_Execute ( $ query , $ params = null )
{
    Global  AOP ;
    if ( isset ( $ params )) {
        $ stmt  =  $ PDO -> prepare ( $ query );
        $ stmt -> execute ( $ params );
    } else {
        $ PDO -> query ( $ query );
    }
}
fonction  PDO_LastInsertId ()
{
    Global  AOP ;
    return  $ PDO -> lastInsertId ();
}
? >
