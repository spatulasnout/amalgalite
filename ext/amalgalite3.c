/**
 * Copyright (c) 2008 Jeremy Hinegardner
 * All rights reserved.  See LICENSE and/or COPYING for details.
 *
 * vim: shiftwidth=4 
 */ 

#include "amalgalite3.h"

/* Module and Classes */
VALUE mA;              /* module Amalgalite                     */
VALUE mAS;             /* module Amalgalite::SQLite3            */
VALUE mASV;            /* module Amalgalite::SQLite3::Version   */
VALUE eAS_Error;       /* class  Amalgalite::SQLite3::Error     */
VALUE cAS_Stat;        /* class  Amalgalite::SQLite3::Stat      */

/*----------------------------------------------------------------------
 * module methods for Amalgalite::SQLite3
 *---------------------------------------------------------------------*/

/*
 * call-seq:
 *    Amalgalite::SQLite3.threadsafe? -> true or false
 *
 * Has the SQLite3 extension been compiled "threadsafe".  If threadsafe? is
 * true then the internal SQLite mutexes are enabled and SQLite is threadsafe.
 * That is threadsafe within the context of 'C' threads.
 *
 */
VALUE am_sqlite3_threadsafe(VALUE self)
{
    if (sqlite3_threadsafe()) {
        return Qtrue;
    } else {
        return Qfalse;
    }
}

/*
 * call-seq:
 *    Amalgalite::SQLite3.complete?( ... , opts = { :utf16 => false }) -> True, False
 *
 * Is the text passed in as a parameter a complete SQL statement?  Or is
 * additional input required before sending the SQL to the extension.  If the
 * extra 'opts' parameter is used, you can send in a UTF-16 encoded string as
 * the SQL.
 *
 * A complete statement must end with a semicolon.
 *
 */
VALUE am_sqlite3_complete(VALUE self, VALUE args)
{
    VALUE sql      = rb_ary_shift( args );
    VALUE opts     = rb_ary_shift( args );
    VALUE utf16    = Qnil;
    int   result = 0;

    if ( ( Qnil != opts ) && ( T_HASH == TYPE(opts) ) ){
        utf16 = rb_hash_aref( opts, rb_intern("utf16") );
    }

    if ( (Qfalse == utf16) || (Qnil == utf16) ) {
        result = sqlite3_complete( StringValuePtr( sql ) );
    } else {
        result = sqlite3_complete16( (void*) StringValuePtr( sql ) );
    }

    return ( result > 0 ) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *    Amalgalite::SQLite3::Stat.update!( reset = false ) -> nil
 *
 * Populates the _@current_ and _@higwater_ instance variables of self
 * object with the values from the sqlite3_status call.  If reset it true then
 * the highwater mark for the stat is reset
 *
 */
VALUE am_sqlite3_stat_update_bang( int argc, VALUE *argv, VALUE self )
{
    int status_op  = -1;
    int current    = -1;
    int highwater  = -1;
    VALUE reset    = Qfalse;
    int reset_flag = 0;
    int rc;

    status_op  = FIX2INT( rb_iv_get( self, "@code" ) );
    if ( argc > 0 ) {
        reset = argv[0];
        reset_flag = ( Qtrue == reset ) ? 1 : 0 ;
    }

    rc = sqlite3_status( status_op, &current, &highwater, reset_flag );

    if ( SQLITE_OK != rc ) {
        VALUE n    = rb_iv_get( self,  "@name" ) ;
        char* name = StringValuePtr( n );
        rb_raise(eAS_Error, "Failure to retrieve status for %s : [SQLITE_ERROR %d] \n", name, rc);
    }

    rb_iv_set( self, "@current", INT2NUM( current ) );
    rb_iv_set( self, "@highwater", INT2NUM( highwater) );

    return Qnil;
}

/*
 * call-seq:
 *    Amalgalite::SQLite3.randomness( N ) -> String of length N
 *
 * Generate N bytes of random data.
 *
 */
VALUE am_sqlite3_randomness(VALUE self, VALUE num_bytes)
{
    int n     = NUM2INT(num_bytes);
    char *buf = ALLOCA_N(char, n);

    sqlite3_randomness( n, buf );
    return rb_str_new( buf, n );
}

/*----------------------------------------------------------------------
 * module methods for Amalgalite::SQLite3::Version
 *---------------------------------------------------------------------*/

/*
 * call-seq:
 *    Amalgalite::SQLite3::Version.to_s -> String
 *
 * Return the SQLite C library version number as a string
 *
 */
VALUE am_sqlite3_libversion(VALUE self)
{
    return rb_str_new2(sqlite3_libversion());
}

/*
 * call-seq:
 *    Amalgalite::SQLite3.Version.to_i -> Fixnum
 *
 * Return the SQLite C library version number as an integer
 *
 */
VALUE am_sqlite3_libversion_number(VALUE self)
{
    return INT2FIX(sqlite3_libversion_number());
}

/**
 * Document-class: Amalgalite::SQLite3
 *
 * The SQLite ruby extension inside Amalgalite.
 *
 */

void Init_amalgalite3()
{
    /*
     * top level module encapsulating the entire Amalgalite library
     */
    mA   = rb_define_module("Amalgalite");

    mAS  = rb_define_module_under(mA, "SQLite3");
    rb_define_module_function(mAS, "threadsafe?", am_sqlite3_threadsafe, 0);
    rb_define_module_function(mAS, "complete?", am_sqlite3_complete, -2);
    rb_define_module_function(mAS, "randomness", am_sqlite3_randomness,1);

    /*
     * class encapsulating a single Stat
     */
    cAS_Stat = rb_define_class_under(mAS, "Stat", rb_cObject);
    rb_define_method(cAS_Stat, "update!", am_sqlite3_stat_update_bang, -1);

    /* 
     * Base class of all SQLite3 errors
     */
    eAS_Error = rb_define_class_under(mAS, "Error", rb_eStandardError); /* in amalgalite.c */

    /**
     * Encapsulation of the SQLite C library version
     */
    mASV = rb_define_module_under(mAS, "Version");
    rb_define_module_function(mASV, "to_s", am_sqlite3_libversion, 0); /* in amalgalite3.c */
    rb_define_module_function(mASV, "to_i", am_sqlite3_libversion_number, 0); /* in amalgalite3.c */

    /*
     * Initialize the rest of the module
     */
    Init_amalgalite3_constants( );
    Init_amalgalite3_database( );
    Init_amalgalite3_statement( );
    Init_amalgalite3_blob( );
    Init_amalgalite3_requires_bootstrap( );

 }


