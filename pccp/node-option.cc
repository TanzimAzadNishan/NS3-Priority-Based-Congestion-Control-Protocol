/*------------------------- my changes starts ---------------------------*/

#include "node-option.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NodeOption");
NS_OBJECT_ENSURE_REGISTERED (NodeOption);

NodeOption::NodeOption()
: m_pktsPerSec (0),
  m_skippedPkts (0),
  m_icn (0),
  m_pktsSinceLastICD (0),
  m_Ta (0),
  m_Ts (0),
  m_parentOffsprings (0),
  m_sp (0),
  m_gp (0),
  m_svc (0.0),
  m_Offsprings (0),
  m_parentDegree (0)
{

}

NodeOption::~NodeOption(){}


TypeId NodeOption::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NodeOption")
    .SetParent<Object> ()
    .SetGroupName ("Internet")
    .AddConstructor<NodeOption> ()
  ;
  return tid;
}


TypeId NodeOption::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


void NodeOption::Print (std::ostream &os) const
{
  os << "pktsPerSec=" << m_pktsPerSec << " "
     << "skippedPkts=" << m_skippedPkts << " "
     << "icn=" << m_icn << " "
     << "pktsSinceLastICD=" << m_pktsSinceLastICD << " "
     << "parentTa=" << m_Ta << " "
     << "parentTs=" << m_Ts << " "
     << "parentOffsprings" << m_parentOffsprings << " "
     << "sp=" << m_sp << " "
     << "gp=" << m_gp << " ";
}

int NodeOption::GetSerializedSize (void) const
{
  return 19;
}

void NodeOption::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;

  i.WriteU8 (20); //length
  i.WriteHtonU16(m_pktsPerSec);
  i.WriteU8(m_skippedPkts); 
  i.WriteU8(m_icn); 
  i.WriteHtonU16(m_pktsSinceLastICD);
  i.WriteHtonU32(m_Ta);
  i.WriteHtonU32(m_Ts);
  i.WriteU8(m_parentOffsprings);
  i.WriteU8(m_sp);
  i.WriteU8(m_gp);
}

int NodeOption::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;

  int length = i.ReadU8();
  if (length != GetSerializedSize())
  {
    NS_LOG_WARN ("Malformed Ipv4 Header option");
    return 0;
  }

  m_pktsPerSec = i.ReadNtohU16();
  m_skippedPkts = i.ReadU8();
  m_icn = i.ReadU8();
  m_pktsSinceLastICD = i.ReadNtohU16();
  m_Ta = i.ReadNtohU32();
  m_Ts = i.ReadNtohU32();
  m_parentOffsprings = i.ReadU8();
  m_sp = i.ReadU8();
  m_gp = i.ReadU8();  

  return GetSerializedSize ();
}


void NodeOption::SetPktsPerSec(int pktsPerSec){
  m_pktsPerSec = pktsPerSec;
}

void NodeOption::SetSkippedPkts(int skippedPkts){
  m_skippedPkts = skippedPkts;
}

void NodeOption::SetIcn(int icn){
  m_icn = icn;
}

void NodeOption::SetPktsSinceLastICD(int pktsSinceLastICD){
  m_pktsSinceLastICD = pktsSinceLastICD;
}

void NodeOption::SetTa(double ta){
  m_Ta = ta;
}

void NodeOption::SetTs(double ts){
  m_Ts = ts;
}

void NodeOption::SetParentOffsprings(int parentOffsprings){
  m_parentOffsprings = parentOffsprings;
}

void NodeOption::SetSP(int sp){
  m_sp = sp;
}

void NodeOption::SetGP(int gp){
  m_gp = gp;
}

void NodeOption::SetSvc(double svc){
  m_svc = svc;
}

void NodeOption::SetOffsprings(int offsprings){
  m_Offsprings = offsprings;
}

void NodeOption::SetParentDegree(double parentDegree){
  m_parentDegree = parentDegree;
}

void NodeOption::SetPktFactor(double pktFactor){
  m_pktFactor = pktFactor;
}

void NodeOption::SetParentAddress(Address parentAddress){
  m_parentAddress = parentAddress;
}


int NodeOption:: GetPktsPerSec (void) const{ 
  return m_pktsPerSec;
}

int NodeOption:: GetSkippedPkts(void) const{
  return m_skippedPkts;
}

int NodeOption:: GetIcn(void) const{
  return m_icn;
}

int NodeOption:: GetPktsSinceLastICD(void) const{
  return m_pktsSinceLastICD;
}

double NodeOption:: GetTa(void) const{
  return m_Ta;
}

double NodeOption:: GetTs(void) const{
  return m_Ts;
}


int NodeOption:: GetParentOffsprings(void) const{
  return m_parentOffsprings;
}

int NodeOption:: GetSP(void) const{
  return m_sp;
}

int NodeOption:: GetGP(void) const{
  return m_gp;
}

double NodeOption::GetSvc(void) const{
  return m_svc;
}

int NodeOption::GetOffsprings(void) const{
  return m_Offsprings;
}

double NodeOption::GetParentDegree(void) const{
  return m_parentDegree;
}

double NodeOption::GetPktFactor(void) const{
  return m_pktFactor;
}

Address NodeOption::GetParentAddress(void) const{
  return m_parentAddress;
}



}  // namespace ns3

/*------------------------- my changes ends ----------------------------*/