//
// FILE: mixedsol.cc -- Mixed strategy solution classes
//
// $Id$
//


#include "mixedsol.h"


gString NameNfgAlgType(NfgAlgType i)
{
  switch(i)
  {
  case NfgAlg_USER:
    return "User"; 
  case NfgAlg_GOBIT: // GobitSolve
    return "Gobit"; 
  case NfgAlg_LIAP: // LiapSolve
    return "Liap"; 
  case NfgAlg_LEMKE: // LcpSolve
    return "LCP(Lemke)"; 
  case NfgAlg_ZSUM: // LpSolve
    return "LP(ZSum)"; 
  case NfgAlg_ENUM: // EnumMixedSolve
    return "Enum"; 
  case NfgAlg_GOBITGRID: // GobitGridSOlve
    return "GobitGrid"; 
  case NfgAlg_SIMPDIV:  // SimpDivSolve
    return "SimpDiv"; 
  case NfgAlg_PURENASH: // EnumPureSolve
    return "PureNash"; 
  case NfgAlg_SEQFORM: // LcpSolve
    return "LCP(Seq)"; 
  default:
    return "ERROR" ;
  }
}


void DisplayNfgAlgType(gOutput& o, NfgAlgType i)
{
  o << NameNfgAlgType(i);
}

