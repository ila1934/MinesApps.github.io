from pathlib import Path
import shutil
root = Path('game/Sources/Simulation')
def edit(name, old, new, count=1):
    path = root / name
    text = path.read_text()
    assert text.count(old) == count, (name, old, text.count(old))
    path.write_text(text.replace(old, new), newline='\n')

for name in ['ShipContacts.h', 'WorldContacts.inc']:
    shutil.copyfile(Path('multiship') / name, root / 'Physics' / name)
edit('Physics/Ship.h', '    FloatSize GetWorldSize() const { return mWorldSize; }',
     '    FloatSize GetWorldSize() const { return mWorldSize; }\n    float GetContactRadius() const { return mContactRadius; }')
edit('Physics/Ship.h', '    FloatSize const mWorldSize;', '    FloatSize const mWorldSize;\n    float const mContactRadius;')
edit('Physics/Ship.h', '        FloatSize const & worldSize,', '        FloatSize const & worldSize,\n        float contactRadius,')
edit('Physics/Ship.cpp', '    FloatSize const & worldSize,', '    FloatSize const & worldSize,\n    float contactRadius,')
edit('Physics/Ship.cpp', '    , mWorldSize(worldSize)', '    , mWorldSize(worldSize)\n    , mContactRadius(contactRadius)')
edit('ShipFactory.cpp', '        shipSize.ToFloat<FloatSize>(),', '        shipSize.ToFloat<FloatSize>(),\n        shipSpaceToWorldSpaceFactor * 0.55f,')
edit('Physics/World.cpp', '#include "Physics.h"', '#include "Physics.h"\n#include "ShipContacts.h"')
edit('Physics/World.cpp', 'namespace Physics {', 'namespace Physics {\n\n#include "WorldContacts.inc"')
# Clamp manual translation, including when the simulation is paused.
path = root / 'Physics/World.cpp'
text = path.read_text()
start = text.index('void World::MoveBy(')
end = text.index('void World::RotateBy(', start)
block = text[start:end]
for signature, component in [
    ('    GlobalConnectedComponentId connectedComponentId,', 'connectedComponentId.GetLocalObjectId()'),
    ('    ShipId shipId,', 'std::nullopt')
]:
    pos = block.index(signature)
    insert = block.index('    // Ship', pos)
    block = block[:insert] + (
        '    vec2f const contactOffset = moveOffset * LimitShipTranslation(\n'
        '        mAllShips, shipId, ' + component + ', moveOffset);\n\n'
    ) + block[insert:]
block = block.replace('        moveOffset,', '        contactOffset,')
text = text[:start] + block + text[end:]
path.write_text(text, newline='\n')
edit('Physics/World.cpp',
    '    for (auto & ship : mAllShips)\n    {\n        ship->Update(',
    '''    PreviousPositions previous;
    if (mAllShips.size() > 1)
    {
        previous.reserve(mAllShips.size());
        for (auto const & ship : mAllShips)
        {
            auto const & points = ship->GetPoints();
            auto & positions = previous.emplace_back();
            positions.reserve(points.GetRawShipPointCount());
            for (auto p : points.RawShipPoints())
                positions.push_back(points.GetPosition(p));
        }
    }

    for (auto & ship : mAllShips)
    {
        ship->Update(''')
edit('Physics/World.cpp',
    '        mNpcs->Update(mCurrentSimulationTime,',
    '''        if (!previous.empty())
        {
            ResolveShipContacts(mAllShips, previous);
            // NPC and rendering broad phases must observe the corrected geometry.
            mAllShipExternalAABBs.Clear();
            for (auto const & ship : mAllShips)
                for (auto const & box : ship->CalculateExternalAABBs().GetItems())
                    mAllShipExternalAABBs.Add(box);
        }
        mNpcs->Update(mCurrentSimulationTime,''')
# The temporary AABB set must outlive iteration over its item range.
edit('Physics/World.cpp',
    '''            for (auto const & ship : mAllShips)
                for (auto const & box : ship->CalculateExternalAABBs().GetItems())
                    mAllShipExternalAABBs.Add(box);''',
    '''            for (auto const & ship : mAllShips)
            {
                auto const boxes = ship->CalculateExternalAABBs();
                for (auto const & box : boxes.GetItems())
                    mAllShipExternalAABBs.Add(box);
            }''')
print('Ship contacts integrated into physics updates and both move-tool paths')
