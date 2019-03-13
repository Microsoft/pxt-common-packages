const enum ControllerGesture {
    //% block="shake"
    Shake = Gesture.Shake,
}

namespace controller {
    
    let lastGesture: ControllerGesture = undefined;
    let gestureHandlers: any;
    /**
     * Do something when a gesture happens (like shaking the board).
     * @param gesture the type of gesture to track, eg: Gesture.Shake
     * @param body code to run when gesture is raised
     */
    //% blockId=ctrlongesture block="on |%NAME"
    //% parts="accelerometer"
    //% gesture.fieldEditor="gridpicker"
    //% gesture.fieldOptions.width=220
    //% gesture.fieldOptions.columns=3
    //% weight=92 blockGap=12
    //% group="Extras"
    export function onGesture(gesture: ControllerGesture, handler: () => void) {
        if (!gestureHandlers) gestureHandlers = {};
        gestureHandlers[gesture] = handler;
        input.onGesture(<Gesture><number>gesture, () => {
            lastGesture = gesture;
        })
    }

    /**
     * Get the acceleration value in milli-gravitys (when the board is laying flat with the screen up,
     * x=0, y=0 and z=-1023)
     * @param dimension TODO
     */
    //% blockId=ctrlaccelerationvalue block="acceleration (mg)|%NAME"
    //% parts="accelerometer"
    //% dimension.fieldEditor="gridpicker"
    //% dimension.fieldOptions.width=180
    //% dimension.fieldOptions.columns=2
    //% weight=42 blockGap=8
    //% group="Extras"
    export function acceleration(dimension: Dimension): number {
        return input.acceleration(dimension);
    }

    function updateGesture() {
        if (gestureHandlers && lastGesture !== undefined && gestureHandlers[lastGesture]) {
            const handler = gestureHandlers[lastGesture];
            lastGesture = undefined;
            handler();
        }
    }

    function initAccelerometer(s: scene.Scene) {
        s.eventContext.registerFrameHandler(scene.UPDATE_PRIORITY, updateGesture);
    }

    scene.Scene.initializers.push(initAccelerometer);
}