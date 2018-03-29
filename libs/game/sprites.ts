/*
Frame handlers:
 10 - physics and collisions
 20 - frame()
 60 - screen/sprite background
 90 - drawing sprites
 95 - drawing score
100 - loops.menu()
200 - screen refresh
*/

/**
 * Sprites on screen
 */
//% weight=98 color="#23c47e" icon="\uf1d8"
//% groups='["Create", "Properties", "Collisions", "Lifecycle"]'
namespace sprites {

    /**
     * Creates a new sprite from an image
     * @param img the image
     */
    //% group="Create"
    //% blockId=spritescreate block="sprite %img"
    //% img.fieldEditor="sprite"
    //% img.fieldOptions.taggedTemplate="img"
    //% blockSetVariable
    //% weight=100
    export function create(img: Image): Sprite {
        game.init()
        let spr = new Sprite(img)
        game.scene.allSprites.push(spr)
        spr.id = game.scene.allSprites.length
        game.scene.physicsEngine.addSprite(spr);
        return spr
    }

    /**
     * Creates a sprite from a sequence of images
     * @param imgs an array of images
     */
    export function createWithAnimation(imgs: Image[]) {
        let s = create(imgs[0])
        s.animation = new SpriteAnimation(imgs)
        return s
    }

    /**
     * Create a new sprite with given speed, and place it at the edge of the screen so it moves towards the middle.
     * The sprite auto-destroys when it leaves the screen. You can modify position after it's created.
     */
    //% group="Create"
    //% blockId=spritescreateprojectile block="projectile %img vx %vx vy %vy"
    //% img.fieldEditor="sprite"
    //% img.fieldOptions.taggedTemplate="img"
    //% weight=99
    //% blockSetVariable
    export function createProjectile(img: Image, vx: number, vy: number) {
        let s = create(img)
        s.vx = vx
        s.vy = vy

        // put it at the edge of the screen so that it moves towards the middle

        if (vx < 0)
            s.x = screen.width + (s.width >> 1) - 1
        else if (vx > 0)
            s.x = -(s.width >> 1) + 1

        if (vy < 0)
            s.y = screen.height + (s.height >> 1) - 1
        else if (vy > 0)
            s.y = -(s.height >> 1) + 1

        s.flags |= sprites.Flag.AutoDestroy;

        return s
    }

    export enum Flag {
        Ghost = 1, // doesn't collide with other sprites
        Destroyed = 2,
        AutoDestroy = 4, // remove the sprite when no longer visible
    }
}
