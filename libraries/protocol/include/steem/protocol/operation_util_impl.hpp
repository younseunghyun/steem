
#pragma once

#include <steem/protocol/operation_util.hpp>

#include <fc/static_variant.hpp>

namespace fc
{
   std::string name_from_type( const std::string& type_name );

   struct from_operation
   {
      variant& var;
      from_operation( variant& dv )
         : var( dv ) {}

      typedef void result_type;
      template<typename T> void operator()( const T& v )const
      {
         auto name = name_from_type( fc::get_typename< T >::name() );
         var = mutable_variant_object( "type", name )( "value", v );
      }
   };

   struct get_operation_name
   {
      string& name;
      get_operation_name( string& dv )
         : name( dv ) {}

      typedef void result_type;
      template< typename T > void operator()( const T& v )const
      {
         name = name_from_type( fc::get_typename< T >::name() );
      }
   };
}

namespace steem { namespace protocol {

struct operation_validate_visitor
{
   typedef void result_type;
   template<typename T>
   void operator()( const T& v )const { v.validate(); }
};

} } // steem::protocol

//
// Place STEEM_DEFINE_OPERATION_TYPE in a .cpp file to define
// functions related to your operation type
//
#define STEEM_DEFINE_OPERATION_TYPE( OperationType )                       \
namespace fc {                                                             \
                                                                           \
void to_variant( const OperationType& var,  fc::variant& vo )              \
{                                                                          \
   var.visit( from_operation( vo ) );                                      \
}                                                                          \
                                                                           \
void from_variant( const fc::variant& var,  OperationType& vo )            \
{                                                                          \
   static std::map<string,uint32_t> to_tag = []()                          \
   {                                                                       \
      std::map<string,uint32_t> name_map;                                  \
      for( int i = 0; i < OperationType::count(); ++i )                    \
      {                                                                    \
         OperationType tmp;                                                \
         tmp.set_which(i);                                                 \
         string n;                                                         \
         tmp.visit( get_operation_name(n) );                               \
         name_map[n] = i;                                                  \
      }                                                                    \
      return name_map;                                                     \
   }();                                                                    \
                                                                           \
   FC_ASSERT( var.is_object(), "Operation has to treated as object." );    \
   auto v_object = var.get_object();                                       \
   FC_ASSERT( v_object.contains( "type" ), "Type field doesn't exist." );  \
   FC_ASSERT( v_object.contains( "value" ), "Value field doesn't exist." );\
                                                                           \
   auto itr = to_tag.find( v_object[ "type" ].as_string() );               \
   FC_ASSERT( itr != to_tag.end(), "Invalid operation name: ${n}", ("n", v_object[ "type" ]) ); \
   vo.set_which( to_tag[ v_object[ "type" ].as_string() ] );               \
   vo.visit( fc::to_static_variant( v_object[ "value" ] ) );               \
} }                                                                        \
                                                                           \
namespace steem { namespace protocol {                                      \
                                                                           \
void operation_validate( const OperationType& op )                         \
{                                                                          \
   op.visit( steem::protocol::operation_validate_visitor() );               \
}                                                                          \
                                                                           \
void operation_get_required_authorities( const OperationType& op,          \
                                         flat_set< account_name_type >& active,         \
                                         flat_set< account_name_type >& owner,          \
                                         flat_set< account_name_type >& posting,        \
                                         std::vector< authority >& other )     \
{                                                                          \
   op.visit( steem::protocol::get_required_auth_visitor( active, owner, posting, other ) ); \
}                                                                          \
                                                                           \
} } /* steem::protocol */
