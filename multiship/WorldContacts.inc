namespace {
using ShipList = std::vector<std::unique_ptr<Ship>>;
using PreviousPositions = std::vector<std::vector<vec2f>>;

float LimitShipTranslation(ShipList const & ships, ShipId id,
    std::optional<ConnectedComponentId> component, vec2f offset)
{
    if (ships.size() < 2 || offset.squareLength() == 0)
        return 1.0f;
    auto const & source = *ships[id];
    auto const & points = source.GetPoints();
    ShipContacts::Sweep moving(source.GetContactRadius());
    for (auto i : points.RawShipPoints())
        if (!component || points.GetConnectedComponentId(i) == *component)
            moving.Add(i, points.GetPosition(i), points.GetPosition(i) + offset);

    float fraction = 1.0f;
    for (auto const & other : ships)
    {
        if (other->GetId() == id) continue;
        ShipContacts::Sweep target(other->GetContactRadius());
        auto const & fixed = other->GetPoints();
        for (auto j : fixed.RawShipPoints())
            target.Add(j, fixed.GetPosition(j), fixed.GetPosition(j));
        ShipContacts::ForEach(moving, target, [&](auto const &, auto const &, auto const & hit) {
            if (offset.dot(hit.normal) < 0)
                fraction = std::min(fraction, hit.time);
        });
    }
    return fraction;
}

void ResolveShipContacts(ShipList const & ships, PreviousPositions const & previous)
{
    std::vector<ShipContacts::Sweep> sweeps;
    for (size_t i = 0; i < ships.size(); ++i)
    {
        auto & sweep = sweeps.emplace_back(ships[i]->GetContactRadius());
        auto const & points = ships[i]->GetPoints();
        sweep.particles.reserve(points.GetRawShipPointCount());
        for (auto p : points.RawShipPoints())
            sweep.Add(p, previous[i][p], points.GetPosition(p));
    }

    struct Contact { float time; size_t shipA, shipB; ElementIndex p, q; };
    std::vector<Contact> contacts;
    for (size_t a = 0; a < ships.size(); ++a)
        for (size_t b = a+1; b < ships.size(); ++b)
            ShipContacts::ForEach(sweeps[a], sweeps[b], [&](auto const & p, auto const & q, auto const & hit) {
                contacts.push_back({hit.time,a,b,p.index,q.index});
            });
    std::stable_sort(contacts.begin(), contacts.end(), [](auto const & a, auto const & b) { return a.time < b.time; });
    for (auto const & contact : contacts)
    {
        auto const p = contact.p, q = contact.q;
        auto & a = ships[contact.shipA]->GetPoints();
        auto & b = ships[contact.shipB]->GetPoints();
        auto pa = a.GetPosition(p), pb = b.GetPosition(q);
        auto va = a.GetVelocity(p), vb = b.GetVelocity(q);
        float const radius = sweeps[contact.shipA].radius + sweeps[contact.shipB].radius;
        ShipContacts::Hit hit;
        if (!ShipContacts::Detect(previous[contact.shipA][p]-previous[contact.shipB][q],pa-pb,radius,hit))
            continue;
        ShipContacts::Respond(hit.normal,radius,
            a.IsPinned(p) ? 0.0f : 1.0f/a.GetMass(p),
            b.IsPinned(q) ? 0.0f : 1.0f/b.GetMass(q),pa,pb,va,vb);
        a.SetPosition(p,pa);
        b.SetPosition(q,pb);
        a.SetVelocity(p,va);
        b.SetVelocity(q,vb);
    }
}
}
