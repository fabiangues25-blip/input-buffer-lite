// Input Buffer Lite - SIN PROBAR. Pensado para Geode v4.x / GD 2.2081.
#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(BufferLayer, GJBaseGameLayer) {
    struct Fields {
        bool pending = false;
        bool pendingRelease = false;
        int waited = 0;
        int button = 1;
        bool isP1 = true;
        bool replaying = false;
    };

    static bool canJump(PlayerObject* p) {
        if (!p) return false;
        return p->m_isOnGround || !p->m_touchingRings.empty();
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();
        if (f->replaying) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }
        auto* p = isPlayer1 ? m_player1 : m_player2;

        if (down && !canJump(p)) {
            f->pending = true;
            f->pendingRelease = false;
            f->waited = 0;
            f->button = button;
            f->isP1 = isPlayer1;
            return;
        }
        if (!down && f->pending && f->isP1 == isPlayer1) {
            f->pendingRelease = true;
            return;
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt) {
        GJBaseGameLayer::processCommands(dt);

        auto f = m_fields.self();
        if (!f->pending) return;

        auto* p = f->isP1 ? m_player1 : m_player2;
        int maxTicks = Mod::get()->getSettingValue<int64_t>("max-ticks");

        if (canJump(p)) {
            f->replaying = true;
            GJBaseGameLayer::handleButton(true, f->button, f->isP1);
            if (f->pendingRelease) {
                GJBaseGameLayer::handleButton(false, f->button, f->isP1);
            }
            f->replaying = false;
            f->pending = false;
            f->pendingRelease = false;
        } else if (++f->waited > maxTicks) {
            f->pending = false;
            f->pendingRelease = false;
        }
    }
};
