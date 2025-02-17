#ifndef GUN_COMPONENT
#define GUN_COMPONENT

#include "bullet_actor.h"
#include "ship_camera_component.h"

// cmx
#include <cmx/cmx_component.h>

struct GunInfo
{
    float timeBetweenShots{.2f};
    int ammoCapacity{50};
    BulletInfo bulletInfo;
};

class GunComponent : public cmx::Component
{
  public:
    GunComponent() = default;
    ~GunComponent() = default;

    CLONEABLE(GunComponent)

    void onAttach() override;
    void update(float dt) override;

    void shoot();
    void reload();

    void setGunInfo(const GunInfo &gunInfo)
    {
        _gunInfo = gunInfo;
    }

    static GunInfo enemyDefaultGun;
    static GunInfo gattlingGun;

  protected:
    float _t{0.f};
    int _ammunitions{0};

    GunInfo _gunInfo;

    std::weak_ptr<class ShipCameraComponent> _cameraComponent;
};

REGISTER_COMPONENT(GunComponent)

#endif
