/**
 * A data logging framework
 */
//% weight=80 color=#0fbc11 icon=""
namespace datalog {
    /**
     * A storage for datalog data
     */
    export class DatalogStorage {
        constructor() {
        }
        /**
         * Initializes the storage
         */
        init(): void { }
        /**
         * Appends the headers in datalog
         */
        appendHeaders(headers: string[]): void { }
        /**
         * Appends a row of data
         */
        appendRow(values: number[]): void { }
        /**
         * Flushes any buffered data
         */
        flush(): void { }
    }

    let _headers: string[] = undefined;
    let _headersWritten: boolean = false;
    let _row: number[] = undefined;
    let _start: number;
    let _storage: DatalogStorage;
    let _enabled = true;
    let _samplingInterval = -1;
    let _sampleCount = 0;
    let _lastSampleTime = -1;

    function clear() {
        _headers = undefined;
        _row = undefined;
    }

    function initRow() {
        if (!_storage || _row) return;

        if (!_headers) {
            _headers = [];
            _headersWritten = false;
            _start = control.millis();
            _storage.init();
        }
        _row = [];
        _sampleCount = 1;
        _lastSampleTime = control.millis();
        const s = (_lastSampleTime - _start) / 1000;
        addValue("time (s)", s);
    }

    function commitRow() {
        // write row if any data
        if (_row && _row.length > 0 && _storage) {
            // write headers for the first row
            if (!_headersWritten) {
                _storage.appendHeaders(_headers);
                _headersWritten = true;
            }
            // commit row data
            if (_samplingInterval <= 0 || control.millis() - _lastSampleTime >= _samplingInterval) {
                // average data
                if (_sampleCount > 1) {
                    for(let i = 1; i < _row.length; ++i) {
                        _row[i] /= _sampleCount;
                    }
                }
                // append row
                _storage.appendRow(_row);
                // clear values
                _row = undefined;
                _sampleCount = 1;
                _lastSampleTime = -1;
            } else {
                // don't store the data yet
                _sampleCount++;
            }
        }
    }

    /**
     * Starts a row of data
     */
    //% weight=100
    //% blockId=datalogAddRow block="datalog add row"
    export function addRow(): void {
        if (!_enabled || !_storage) return;

        commitRow();
        initRow();
    }

    /**
     * Adds a cell to the row of data
     * @param name name of the cell, eg: "x"
     * @param value value of the cell, eg: 0
     */
    //% weight=99
    //% blockId=datalogAddValue block="datalog add %name|=%value"
    export function addValue(name: string, value: number) {
        if (!_row) return;
        // happy path
        if (_headers[_row.length] === name)
            _row.push(value);
        else {
            let i = _headers.indexOf(name);
            if (i < 0) {
                _headers.push(name);
                i = _headers.length - 1;
            }
            _row[i] += value;
        }
    }

    /**
     * 
     * @param storage custom storage solution
     */
    //%
    export function setStorage(storage: DatalogStorage) {
        flush();
        _storage = storage;
        clear();
    }

    /**
     * Commits any buffered row to disk
     */
    //%
    export function flush() {
        if (_headers && _storage)
            _storage.flush();
    }

    /**
     * Sets the minimum number of milli seconds between rows
     * @param millis milliseconds between each sample, eg: 50
     */
    //% blockId=datalogSetSamplingInterval block="set datalog sampling interval to %millis|(ms)"
    export function setSampleInterval(millis: number) {
        _samplingInterval = millis >> 0;
    }

    /**
     * Turns on or off datalogging
     * @param enabled 
     */
    //% blockId=datalogEnabled block="datalog %enabled=toggleOnOff"
    export function setEnabled(enabled: boolean) {
        flush();
        _enabled = enabled;
    }
}
