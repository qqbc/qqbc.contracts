#include <eosio.system/eosio.system.hpp>


namespace eosiosystem {

   void system_contract::invite( const name& inviter, const std::vector<name>& invitees ) {
        require_auth( inviter );
        check( invitees.size() > 0, "cannot invite no invitees" );
        check( invitees.size() <= qqbc_max_invitee, "attempt to invite for too many invitees" );
        for( size_t i = 0; i < invitees.size(); ++i ) {
            if( i + 1 < invitees.size() ){
                check( invitees[i] < invitees[i + 1], "invitees must be unique and sorted" );
            }
            check( invitees[i] != inviter, "cannot invite self");
        }
        eosio::print("qqbc:", "invite:", "\t inviter:", inviter, "\n");

        for( const auto& i : invitees ) {
            add_inviter(i, inviter);
       }
   }

   void system_contract::add_inviter(const name& invitee, const name& inviter ){
       eosio::print("qqbc:", "add_invitor:", "\t invitee:", invitee, "\t inviter:", inviter, "\n");


       inviters_table     inviter_tbl( get_self(), invitee.value );
       auto iter = inviter_tbl.find(invitee.value );

       // no inviter table
       if( iter == inviter_tbl.end() ) {
           eosio::print("qqbc:", "insert new inviter record", "\n");
           inviter_tbl.emplace( invitee, [&]( auto& iv ){
               iv.owner = invitee;
               iv.pending_inviters.reserve(qqbc_max_invitor);
               iv.pending_inviters.push_back(inviter);
           });
       }
       else {
           eosio::print("qqbc:", "modify inviter record", "\n");

           check( iter->pending_inviters.size()  < qqbc_max_invitor, "attempt to accept too may inviters" );

           //already in the inviter list
           if(std::any_of(iter->pending_inviters.cbegin(), iter->pending_inviters.cend(),
                   [&](const name& n){
                return (n == inviter);
           })){
               eosio::print("qqbc:", "inviter already in the list", "\n");
               return;
           };

           inviter_tbl.modify( iter, same_payer, [&]( auto& iv ){
               iv.pending_inviters.push_back(inviter);
               std::sort( iv.pending_inviters.begin(), iv.pending_inviters.end(), []( const name& lhs, const name& rhs ) {
                   return lhs < rhs;
               } );
           });
       }
   }



} /// namespace eosiosystem
