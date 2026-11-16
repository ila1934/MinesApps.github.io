#pragma once
#include <Core/Vectors.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace Physics::ShipContacts {
struct Box {
    float left = std::numeric_limits<float>::max(), bottom = left;
    float right = std::numeric_limits<float>::lowest(), top = right;
    void Add(vec2f a, vec2f b, float radius) {
        left = std::min(left, std::min(a.x,b.x)-radius);
        right = std::max(right, std::max(a.x,b.x)+radius);
        bottom = std::min(bottom, std::min(a.y,b.y)-radius);
        top = std::max(top, std::max(a.y,b.y)+radius);
    }
    bool Overlaps(Box const & other) const {
        return left <= other.right && right >= other.left && bottom <= other.top && top >= other.bottom;
    }
};
struct Particle { unsigned index; vec2f start, end; Box bounds; };
struct Sweep {
    explicit Sweep(float radius) : radius(radius) {}
    float radius, maxWidth = 0.0f;
    Box bounds;
    std::vector<Particle> particles;
    bool sorted = false;
    void Add(unsigned index, vec2f start, vec2f end) {
        Box box;
        box.Add(start,end,radius);
        bounds.Add(start,end,radius);
        maxWidth = std::max(maxWidth, box.right-box.left);
        particles.push_back({index,start,end,box});
    }
};
struct Hit { float time; vec2f normal; };

// First contact between two moving material points, in relative coordinates.
inline bool Detect(vec2f start, vec2f end, float radius, Hit & hit) {
    vec2f const delta = end-start;
    double const a = delta.dot(delta), b = start.dot(delta);
    double const c = double(start.x)*start.x + double(start.y)*start.y - double(radius)*radius;
    double time = 0.0;
    if (c > 0.0) {
        if (b >= 0 || a == 0) return false;
        double const discriminant = b*b-a*c;
        if (discriminant < 0) return false;
        time = c/(-b+std::sqrt(discriminant));
        if (time > 1.0) return false;
    }
    vec2f const d = start+delta*float(time);
    float const length = d.length();
    hit = {float(time), length > 1e-7f ? d/length : vec2f(1,0)};
    return true;
}
template<typename Callback>
void ForEach(Sweep const & a, Sweep & b, Callback callback) {
    if (!a.bounds.Overlaps(b.bounds)) return;
    if (!b.sorted) {
        std::sort(b.particles.begin(),b.particles.end(),[](auto const & p, auto const & q) {
            return p.bounds.left < q.bounds.left;
        });
        b.sorted = true;
    }
    for (auto const & p : a.particles) {
        if (!p.bounds.Overlaps(b.bounds)) continue;
        auto it = std::lower_bound(b.particles.begin(), b.particles.end(), p.bounds.left-b.maxWidth,
            [](auto const & q, float left) { return q.bounds.left < left; });
        for (; it != b.particles.end() && it->bounds.left <= p.bounds.right; ++it) {
            Hit hit;
            if (p.bounds.Overlaps(it->bounds) && Detect(p.start-it->start,p.end-it->end,a.radius+b.radius,hit))
                callback(p,*it,hit);
        }
    }
}
inline void Respond(vec2f normal, float radius, float invA, float invB,
    vec2f & pa, vec2f & pb, vec2f & va, vec2f & vb) {
    float const sum = invA+invB;
    if (sum <= 0) return;
    float const depth = std::max(0.0f,radius-(pa-pb).dot(normal));
    pa += normal*(depth*invA/sum);
    pb -= normal*(depth*invB/sum);
    float const closing = (va-vb).dot(normal);
    if (closing < 0) {
        float const impulse = -1.04f*closing/sum;
        va += normal*(impulse*invA);
        vb -= normal*(impulse*invB);
    }
}
}
