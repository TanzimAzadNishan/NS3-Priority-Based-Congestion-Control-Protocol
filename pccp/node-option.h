/*------------------------- my changes starts ---------------------------*/

#ifndef IPV4_OPTION_H
#define IPV4_OPTION_H

#include <stdint.h>
#include "ns3/object.h"
#include "ns3/buffer.h"
#include "ns3/object-factory.h"
#include "ns3/address.h"


namespace ns3 {

class NodeOption : public Object{

public:
    NodeOption();
    virtual ~NodeOption();

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;

    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual int Deserialize (Buffer::Iterator start);
    virtual int GetSerializedSize (void) const;

    void SetPktsPerSec(int pktsPerSec);
    void SetSkippedPkts(int skippedPkts);
    void SetIcn(int icn);
    void SetPktsSinceLastICD(int pktsSinceLastICD);
    void SetTa(double ta);
    void SetTs(double ts);
    void SetParentOffsprings(int parentOffsprings);
    void SetSP(int sp);
    void SetGP(int gp);
    void SetSvc(double svc);
    void SetOffsprings(int offsprings);
    void SetParentDegree(double parentDegree);
    void SetPktFactor(double pktFactor);
    void SetParentAddress(Address parentAddress);

    int GetPktsPerSec (void) const;
    int GetSkippedPkts(void) const;
    int GetIcn(void) const;
    int GetPktsSinceLastICD(void) const;
    double GetTa(void) const;
    double GetTs(void) const;
    int GetParentOffsprings(void) const;
    int GetSP(void) const;
    int GetGP(void) const;
    double GetSvc(void) const;
    int GetOffsprings(void) const;
    double GetParentDegree(void) const;
    double GetPktFactor(void) const;
    Address GetParentAddress(void) const;    

private:
    int    m_pktsPerSec;
    int     m_skippedPkts;
    int     m_icn;
    int    m_pktsSinceLastICD;
    double    m_Ta;
    double    m_Ts;
    int     m_parentOffsprings;
    int     m_sp;
    int     m_gp;
    double  m_svc;
    int m_Offsprings;
    double m_parentDegree;
    double m_pktFactor;
    Address m_parentAddress;
};

}  // namespace ns3


#endif /* IPV4_OPTION_H */

/*------------------------- my changes ends ----------------------------*/