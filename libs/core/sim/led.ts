namespace pxsim.visuals {
    type SVGStylable = any;
    
    const LED_PART_XOFF = -8;
    const LED_PART_YOFF = -7;
    const LED_PART_WIDTH = 68;
    const LED_PART_HEIGHT = 160;
    const LED_PART = `<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:cc="http://creativecommons.org/ns#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
    xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
    xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" version="1.1" id="Layer_1" x="0px" y="0px" viewBox="0 0 33.6 79.1"
    style="enable-background:new 0 0 33.6 79.1;" xml:space="preserve" inkscape:version="0.91 r13725" sodipodi:docname="led2.svg">
    <metadata id="metadata73">
        <rdf:RDF>
            <cc:Work rdf:about="">
                <dc:format>image/svg+xml</dc:format>
                <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
            </cc:Work>
        </rdf:RDF>
    </metadata>
    <defs id="defs71" />
    <sodipodi:namedview pagecolor="#ffffff" bordercolor="#666666" borderopacity="1" objecttolerance="10" gridtolerance="10"
        guidetolerance="10" inkscape:pageopacity="0" inkscape:pageshadow="2" inkscape:window-width="909" inkscape:window-height="593"
        id="namedview69" showgrid="false" inkscape:zoom="5.9671303" inkscape:cx="16.632414" inkscape:cy="52.956779" inkscape:window-x="237"
        inkscape:window-y="35" inkscape:window-maximized="0" inkscape:current-layer="g7" />
    <title id="title3">SparkFun Parts for MS Kit</title>
    <path class="st0" style="opacity:0.65; fill:rgb(236,236,236);" d="M1.3,66.1V72c0,3.9,3.2,7.1,7.1,7.1s7.1-3.2,7.1-7.1l0,0V58.3c-1.9-1.9-4.4-2.9-7.1-2.8         c-4.6,0-8.4,2.6-8.4,5.9v1.5C0,64.1,0.5,65.2,1.3,66.1z"
        id="path5" />
    <g id="g7">
        <path class="st1" style="fill:rgb(140,140,140);" d="M12.7,61.6l1.2,1.4h-1l-2.4-1.4V27c0-0.3,0.5-0.5,1.1-0.5l0,0c0.6,0,1.1,0.2,1.1,0.5V61.6z"
            id="path9" />
        <path class="st1" style="fill:rgb(140,140,140);" d="M2.6,54.9c0,0.7,1.1,1.3,2.1,1.8c0.4,0.2,1.2,0.6,1.2,0.9V61l-2.5,2h0.9L8,61v-3.5c0-0.7-0.9-1.2-1.9-1.7             c-0.4-0.2-1.3-0.8-1.3-1.1c0-0.1,0-46.8,0-52.9c0-0.4-0.5-0.7-1.1-0.7l0,0l0,0c-0.6,0-1.1,0.3-1.1,0.7l0,0L2.6,54.9z"
            id="path11" />
        <path class="sim-led-main" style="opacity:0.3;fill:rgb(204,204,204);" d="M1.3,66.1V72c0,3.9,3.2,7.1,7.1,7.1s7.1-3.2,7.1-7.1l0,0V58.3c-1.9-1.9-4.4-2.9-7.1-2.8             c-4.6,0-8.4,2.6-8.4,5.9v1.5C0,64.1,0.5,65.2,1.3,66.1z"
            id="LED" inkscape:label="#path13" />
        <path class="st3" style="opacity:0.9;fill:rgb(209,209,209);" d="M1.3,66.1V63c0-2.7,3.2-5,7.1-5s7.1,2.2,7.1,5v-4.6c-1.9-1.9-4.4-2.9-7.1-2.8c-4.6,0-8.4,2.6-8.4,5.9V63             C0,64.1,0.5,65.2,1.3,66.1z"
            id="path15" />
        <path class="st4" style="opacity:0.7;fill:rgb(230,230,230);" d="M1.3,66.1V63c0-2.7,3.2-5,7.1-5s7.1,2.2,7.1,5v-4.6c-1.9-1.9-4.4-2.9-7.1-2.8c-4.6,0-8.4,2.6-8.4,5.9V63             C0,64.1,0.5,65.2,1.3,66.1z"
            id="path17" />
        <path class="st5" style="opacity:0.25;fill:rgb(230,230,230);" d="M1.3,66.1V63c0-2.7,3.2-5,7.1-5s7.1,2.2,7.1,5v-3.1c-1.9-1.9-4.4-2.9-7.1-2.8C3.8,57.1,0,59.7,0,63             C0,64.1,0.5,65.2,1.3,66.1z"
            id="path19" />
        <ellipse class="st5" style="opacity:0.25;fill:rgb(230,230,230);" cx="8.3" cy="63" rx="7.1" ry="5" id="ellipse21" />
        <ellipse class="st5" style="opacity:0.25;fill:rgb(230,230,230);" cx="8.3" cy="63" rx="7.1" ry="5" id="ellipse23" />
        <g class="st8" style="opacity:0.61;" id="g29">
            <path class="st9" style="fill:rgb(255,255,255);" d="M8.3,57.1c4.3,0,6.1,2,6.1,2l-0.7,0.7c0,0-1.6-1.7-5.4-1.7C5.9,58,3.6,59,2,60.8l-0.8-0.6                 C3.1,58.1,5.6,57,8.3,57.1z"
                id="path31" />
        </g>
        <g class="st8" style="opacity:0.61;" id="g33">
            <path class="st9" style="fill:rgb(255,255,255);" d="M12.9,75.9c1.1-1.1,1.7-2.6,1.7-4.2V61.4l-1.9-1.5v10.4c0.9,2.8,0.3,4.2-0.7,5.2                 C12.3,75.6,12.6,75.7,12.9,75.9z"
                id="path35" />
            <path class="st9" style="fill:rgb(255,255,255);" d="M5.6,77.5c0.1-0.3,0.2-0.6,0.3-0.9c-1.5-0.7-2.6-2.1-2.8-3.7h-1C2.4,74.9,3.7,76.6,5.6,77.5z"
                id="path37" />
        </g>
    </g>
    <g id="g39">
        <rect x="11.6" y="16.9" class="st1" style="fill:rgb(140,140,140);" width="21.7" height="1.9" id="rect41" />
        <g id="g43">
            <rect x="12" y="16.9" class="st10" style="fill:none;" width="3.2" height="1.9" id="rect45" />
            <path class="st11" style="fill:rgb(214,191,144);" d="M19,15c-0.3-0.2-0.6-0.3-0.9-0.3h-1.4c-0.3,0-0.5,0.3-0.5,0.7v4.9c0,0.4,0.2,0.7,0.5,0.7h1.4                 c0.3,0,0.6-0.1,0.9-0.3l0,0c0.3-0.2,0.6-0.3,0.9-0.3h5c0.3,0,0.6,0.1,0.9,0.3l0.1,0c0.3,0.2,0.6,0.3,0.9,0.3h1.4                 c0.3,0,0.5-0.3,0.5-0.7v-4.9c0-0.4-0.2-0.7-0.5-0.7h-1.4c-0.3,0-0.6,0.1-0.9,0.3l-0.1,0c-0.3,0.2-0.6,0.3-0.9,0.3h-5                 C19.7,15.3,19.4,15.2,19,15L19,15z"
                id="path47" />
            <path class="st12" style="fill:rgb(170,147,107);" d="M28.4,18.5c-0.1,0.1-0.1,0.2-0.2,0.3c-0.3,0.5-0.7,0.8-1.2,0.8c-0.5,0-0.9-0.1-1.4-0.3                 c-0.6-0.1-1.1-0.1-1.7-0.1c-2,0-3.9,0-5.9,0.2c-0.4,0.1-0.8,0-1.1-0.1c-0.2-0.1-0.4-0.2-0.5-0.5v1.5c0,0.2,0.1,0.3,0.2,0.3h1.4                 c0.3,0,0.6-0.1,0.9-0.3c0.3-0.2,0.7-0.3,1.1-0.3h5c0.4,0,0.8,0.1,1.1,0.3c0.3,0.1,0.6,0.2,0.8,0.2h1.4c0.1,0,0.2-0.1,0.2-0.3v-1.9                 C28.5,18.4,28.4,18.5,28.4,18.5z"
                id="path49" />
            <g id="g51">
                <rect x="27.2" y="14.7" class="st13" style="fill:rgb(173,159,78);" width="0.7" height="6.2" id="rect53" />
                <rect x="27.2" y="17.8" class="st14" style="opacity:0.4;" width="0.7" height="2.5" id="rect55" />
                <rect x="27.2" y="15" class="st15" style="opacity:0.5;fill:rgb(255,255,51);" width="0.7" height="1.3" id="rect57" />
                <rect x="27.2" y="15.3" class="st16" style="opacity:0.5;fill:rgb(255,255,255);" width="0.7" height="0.7" id="rect59"
                />
            </g>
            <rect x="23.1" y="15.3" class="st17" style="fill:rgb(170,69,24);" width="1.3" height="5.1" id="rect61" />
            <rect x="20.6" y="15.3" class="st18" style="fill:rgb(255,151,0);" width="1.3" height="5.1" id="rect63" />
            <path class="st18" style="fill:rgb(255,151,0);" d="M19.3,15.1c-0.1,0-0.1-0.1-0.2-0.1l0,0c-0.3-0.2-0.6-0.3-0.9-0.3H18V21h0.1c0.3,0,0.6-0.1,0.9-0.3l0,0                 c0.1,0,0.1-0.1,0.2-0.1V15.1z"
                id="path65" />
            <path class="st19" style="opacity:0.74;fill:rgb(255,253,250);" d="M18.7,15.7c0.4,0.1,0.8,0.2,1.2,0.2c0.4,0,0.7,0,1.1,0c1.2-0.1,2.4-0.1,3.6,0c0.4,0,0.9,0,1.3-0.1                 c0.3-0.1,0.6-0.2,0.8-0.3c0.6-0.2,1.2-0.3,1.8-0.2c0-0.1-0.1-0.3-0.2-0.3h-1.4c-0.3,0-0.6,0.1-0.9,0.3c-0.3,0.2-0.7,0.3-1.1,0.3                 h-5c-0.4,0-0.8-0.1-1.1-0.3c-0.3-0.1-0.6-0.2-0.8-0.2h-1.4c-0.1,0-0.2,0.1-0.2,0.3v0.2C17.2,15.5,17.9,15.6,18.7,15.7z"
                id="path67" />
        </g>
    </g>
</svg>`;

    // For the intructions
    export function mkLedPart(xy: Coord = [0, 0]): SVGElAndSize {
        let [x, y] = xy;
        let l = x + LED_PART_XOFF;
        let t = y + LED_PART_YOFF;
        let w = LED_PART_WIDTH;
        let h = LED_PART_HEIGHT;
        let img = <SVGGElement>svg.elt("image");
        svg.hydrate(img, {
            class: "sim-led", x: l, y: t, width: w, height: h,
            href: svg.toDataUri(LED_PART)
        });
        return { el: img, x: l, y: t, w: w, h: h };
    }

    export class LedView implements IBoardPart<EdgeConnectorState> {
        element: SVGElement;
        defs: SVGElement[];

        private led: SVGPathElement;
        private parsePinString: (s: string) => Pin;
        private color: string = "rgb(0,255,0)"; // green color by default

        private part: SVGElAndSize;
        private bus: EventBus;
        public style: string;

        private state: ToggleState;
        private pin: Pin;

        private currentlyOn: boolean = false;
        private currentValue: number;

        constructor(parsePinString:(s: string) => Pin) {
            this.parsePinString = parsePinString;
        }

        public init(bus: EventBus, state: EdgeConnectorState, svgEl: SVGSVGElement, otherParams: Map<string>): void {
            this.pin = this.parsePinString(otherParams["name"] || otherParams["pin"]);
            this.bus = bus;
            this.initDom();
            this.updateState();
        }

        initDom() {
            this.element = svg.elt("g");
            const image = new DOMParser().parseFromString(LED_PART, "image/svg+xml").querySelector("svg") as SVGSVGElement;
            svg.hydrate(image, {
                class: "sim-led", width: LED_PART_WIDTH, height: LED_PART_HEIGHT,
            });
            this.led = image.getElementById('LED') as SVGPathElement;
            this.element.appendChild(image);

        }

        public moveToCoord(xy: Coord) {
            translateEl(this.element, [xy[0] + LED_PART_XOFF, xy[1] + LED_PART_YOFF]);
        }

        public updateTheme() {
        }

        public updateState() {
            if (this.currentValue === this.pin.value) {
                return;
            }

            this.currentValue = this.pin.value;
            (<SVGStylable><any>this.led).style.fill = this.currentValue ? "#00ff00" : "#ffffff";
            (<SVGStylable><any>this.led).style.opacity = "0.9";
        }
    }
}