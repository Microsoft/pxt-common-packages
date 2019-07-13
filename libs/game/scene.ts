interface SparseArray<T> {
    [index: number]: T;
}

/**
 * Control the background, tiles and camera
 */
namespace scene {
    export enum Flag {
        NeedsSorting = 1 << 1,
    }

    export interface SpriteHandler {
        kind: number;
        handler: (sprite: Sprite) => void;
    }

    export interface OverlapHandler {
        kind: number;
        otherKind: number;
        handler: (sprite: Sprite, otherSprite: Sprite) => void;
    }

    export interface CollisionHandler {
        kind: number;
        handler: (sprite: Sprite) => void
    }

    export interface GameForeverHandlers {
        lock: boolean;
        handler: () => void;
    }
    
    class Renderable implements SpriteLike {
        protected _z: number;
        public id: number
        public constructor(
            protected handler: (target: Image, camera: Camera) => void,
            z: number
        ) {
            this.z = z;
            game.currentScene().addSprite(this);
        }

        get z(): number {
            return this._z;
        }

        set z(v: number) {
            if (this._z !== v) {
                this._z = v;
                game.currentScene().flags |= Flag.NeedsSorting;
            }
        }
        
        __draw(camera: scene.Camera) {
            this.handler(screen, camera);
        }

        __update() { }

        __serialize(): Buffer { return undefined }
    }

    export const CONTROLLER_PRIORITY = 8;
    export const TILEMAP_PRIORITY = 9;
    export const PHYSICS_PRIORITY = 10;
    export const ANIMATION_UPDATE_PRIORITY = 15;
    export const UPDATE_CONTROLLER_PRIORITY = 13;
    export const CONTROLLER_SPRITES_PRIORITY = 13;
    export const OVERLAP_PRIORITY = 15;
    export const UPDATE_INTERVAL_PRIORITY = 19;
    export const UPDATE_PRIORITY = 20;
    export const RENDER_BACKGROUND_PRIORITY = 60;
    export const PAINT_PRIORITY = 75;
    export const RENDER_SPRITES_PRIORITY = 90;
    export const SHADE_PRIORITY = 94;
    export const HUD_PRIORITY = 95;
    export const RENDER_DIAGNOSTICS_PRIORITY = 150;
    export const UPDATE_SCREEN_PRIORITY = 200;

    export class Scene {
        eventContext: control.EventContext;
        background: Background;
        tileMap: tiles.TileMap;
        allSprites: SpriteLike[];
        private spriteNextId: number;
        spritesByKind: SparseArray<SpriteSet>;
        physicsEngine: PhysicsEngine;
        camera: scene.Camera;
        flags: number;
        destroyedHandlers: SpriteHandler[];
        createdHandlers: SpriteHandler[];
        overlapHandlers: OverlapHandler[];
        overlapMap: SparseArray<number[]>;
        collisionHandlers: CollisionHandler[][];
        gameForeverHandlers: GameForeverHandlers[];
        particleSources: particles.ParticleSource[];
        controlledSprites: controller.ControlledSprite[][];

        private _millis: number;
        private _data: any;

        // a set of functions that need to be called when a scene is being initialized
        static initializers: ((scene: Scene) => void)[] = [];

        constructor(eventContext: control.EventContext) {
            this.eventContext = eventContext;
            this.flags = 0;
            this.physicsEngine = new ArcadePhysicsEngine();
            this.camera = new scene.Camera();
            this.background = new Background(this.camera);
            this.destroyedHandlers = [];
            this.createdHandlers = [];
            this.overlapHandlers = [];
            this.overlapMap = {};
            this.collisionHandlers = [];
            this.gameForeverHandlers = [];
            this.spritesByKind = {};
            this.controlledSprites = [];
            this._data = {};
            this._millis = 0;
        }

        init() {
            if (this.allSprites) return;

            power.poke(); // keep game alive a little more
            this.allSprites = [];
            this.spriteNextId = 0;
            // update controller state
            this.eventContext.registerFrameHandler(CONTROLLER_PRIORITY, () => {
                this._millis += this.eventContext.deltaTimeMillis;
                control.enablePerfCounter("controller_update")
                controller.__update(this.eventContext.deltaTime);
            })
            // update sprites in tilemap
            this.eventContext.registerFrameHandler(TILEMAP_PRIORITY, () => {
                control.enablePerfCounter("tilemap_update")
                if (this.tileMap) {
                    this.tileMap.update(this.camera);
                }
            })
            // controller update 13
            this.eventContext.registerFrameHandler(CONTROLLER_SPRITES_PRIORITY, controller._moveSprites);
            // apply physics and collisions 15
            this.eventContext.registerFrameHandler(OVERLAP_PRIORITY, () => {
                control.enablePerfCounter("physics and collisions")
                const dt = this.eventContext.deltaTime;

                this.physicsEngine.move(dt);
                this.camera.update();

                for (const s of this.allSprites)
                    s.__update(this.camera, dt);
            })
            // user update 20
            // render background 60
            this.eventContext.registerFrameHandler(RENDER_BACKGROUND_PRIORITY, () => {
                control.enablePerfCounter("render background")
                this.background.draw();
            })
            // paint 75
            // render sprites 90
            this.eventContext.registerFrameHandler(RENDER_SPRITES_PRIORITY, () => {
                this.cachedRender = undefined;
                control.enablePerfCounter("sprite_draw")
                if (this.flags & Flag.NeedsSorting)
                    this.allSprites.sort((a, b) => a.z - b.z || a.id - b.id)
                for (const s of this.allSprites)
                    s.__draw(this.camera);
            })
            // render diagnostics
            this.eventContext.registerFrameHandler(RENDER_DIAGNOSTICS_PRIORITY, () => {
                if (game.stats && control.EventContext.onStats) {
                    control.EventContext.onStats(
                        control.EventContext.lastStats +
                        ` sprites:${this.allSprites.length}`
                    )
                }
                if (game.debug)
                    this.physicsEngine.draw();
                game.consoleOverlay.draw();
                // clear flags
                this.flags = 0;
                // check for power deep sleep
                power.checkDeepSleep();
            });
            // update screen
            this.eventContext.registerFrameHandler(UPDATE_SCREEN_PRIORITY, control.__screen.update);
            // register additional components
            Scene.initializers.forEach(f => f(this));
        }

        get data() {
            return this._data;
        }

        /**
         * Gets the elapsed time in the scene
         */
        millis(): number {
            return this._millis;
        }

        addSprite(sprite: SpriteLike) {
            this.allSprites.push(sprite);
            sprite.id = this.spriteNextId++;
        }

        destroy() {
            this.eventContext = undefined;
            this.background = undefined;
            this.tileMap = undefined;
            this.allSprites = undefined;
            this.spriteNextId = undefined;
            this.spritesByKind = undefined;
            this.physicsEngine = undefined;
            this.camera = undefined;
            this.flags = undefined;
            this.destroyedHandlers = undefined;
            this.createdHandlers = undefined;
            this.overlapHandlers = undefined;
            this.collisionHandlers = undefined;
            this.gameForeverHandlers = undefined;
            this._data = undefined;
        }

        registerRenderable(
            handler: (target: Image, camera: Camera) => void,
            z = 100
        ): SpriteLike {
            const renderable = new Renderable(
                handler,
                z
            );

            this.allSprites.push(renderable);
            return renderable;
        }

        /**
         * renders the current frame as an image
         */
        render(): Image {
            if (this.cachedRender) {
                return this.cachedRender;
            }
            // todo: un-dup this from init; either call this from the update, or split into
            // a few helpers
            this.background.draw();

            if (this.flags & Flag.NeedsSorting)
                this.allSprites.sort(function (a, b) { return a.z - b.z || a.id - b.id; })
            for (const s of this.allSprites)
                s.__draw(this.camera);

            // TODO: render game.onShade here
            // TODO: render info values here -- register that as a renderable.
            // TODO: register tilemap as renderable
            this.cachedRender = screen.clone();
            return this.cachedRender;
        }
        cachedRender: Image;
    }
}
