#include "ShipContacts.h"
#include <iostream>
#include <stdexcept>
using namespace Physics::ShipContacts;
void Require(bool condition, char const * message)
{
    if (!condition) throw std::runtime_error(message);
}
int main()
{
    Hit hit;
    Require(Detect(vec2f(-10,0),vec2f(10,0),1,hit), "Fast crossing missed");
    Require(std::abs(hit.time-0.45f)<0.0001f && hit.normal.x<0, "Wrong impact time");
    Require(!Detect(vec2f(-10,3),vec2f(10,3),1,hit), "Distant hulls collided");
    Require(!Detect(vec2f(-2,0),vec2f(-3,0),1,hit), "Separating hulls collided");
    Require(Detect(vec2f(0,0),vec2f(0,0),1,hit), "Overlap missed");
    Require(std::isfinite(hit.normal.x), "Coincident points produced NaN");

    vec2f pa(1,0), pb(-1,0), va(10,0), vb(-10,0);
    Respond(vec2f(-1,0),1,1,1,pa,pb,va,vb);
    Require(pb.x-pa.x>=0.9999f, "Crossing hull points not separated");
    Require(va.x<=0 && vb.x>=0, "Incoming velocities not stopped");
    Require(std::abs(va.x+vb.x)<0.001f, "Momentum not conserved");

    pa=vec2f(0,0);pb=vec2f(0.5f,0);va=vec2f(5,0);vb=vec2f(0,0);
    Respond(vec2f(-1,0),1,0.5f,0,pa,pb,va,vb);
    Require(pb.x==0.5f && vb.x==0, "Pinned target moved");
    Require(pa.x<=-0.4999f && va.x<=0, "Moving point passed pinned target");

    // A row moving ten units per frame must hit a stationary row even though
    // their final positions have no overlap.
    Sweep a(0.55f), b(0.55f);
    for (unsigned i=0;i<40;++i)
    {
        a.Add(i,vec2f(-5,float(i)),vec2f(5,float(i)));
        b.Add(i,vec2f(0,float(i)),vec2f(0,float(i)));
    }
    unsigned count=0;
    float first=1;
    ForEach(a,b,[&](auto const &, auto const &, Hit const & h){++count;first=std::min(first,h.time);});
    Require(count>=40 && first>0.38f && first<0.40f,"Swept broad phase missed hull");

    // Scaled geometry uses scaled collision distances.
    Require(Detect(vec2f(-0.2f,0),vec2f(0.2f,0),0.02f,hit),"Small-scale ship missed");
    Require(std::abs(hit.time-0.45f)<0.001f,"Scale changed time of impact");
    // Separating overlaps must not gain energy.
    pa=vec2f(-0.2f,0);pb=vec2f(0.2f,0);va=vec2f(-3,0);vb=vec2f(3,0);
    Respond(vec2f(-1,0),1,1,1,pa,pb,va,vb);
    Require(va.x==-3 && vb.x==3,"Separating contact gained impulse");
    std::cout << "PASS: fast crossing, near miss, overlap, momentum, pins, broad phase, scale, separation\n";
}
