namespace net {
    export class Response {
        _cached: Buffer
        status_code: number
        reason: string
        _read_so_far: number
        headers: StringMap;

        /** 
         * The response from a request, contains all the headers/content 
         */
        constructor(private socket: Socket) {
            this._cached = null
            this.status_code = null
            this.reason = null
            this._read_so_far = 0
            this.headers = {}
        }

        /** 
         * Close, delete and collect the response data 
         */
        public close() {
            if (this.socket) {
                this.socket.close()
                this.socket = null
            }
            this._cached = null
        }

        /** 
         * The HTTP content direct from the socket, as bytes 
         */
        get content() {
            let content_length = parseInt(this.headers["content-length"]) || 0

            // print("Content length:", content_length)
            if (this._cached === null) {
                this._cached = this.socket.read(content_length)
                this.socket.close()
                this.socket = null
            }

            // print("Buffer length:", len(self._cached))
            return this._cached
        }

        /** 
         * The HTTP content, encoded into a string according to the HTTP header encoding
        */
        get text() {
            return this.content.toString()
        }

        get json() {
            return JSON.parse(this.text)
        }

        public toString() {
            return `HTTP ${this.status_code}; ${Object.keys(this.headers).length} headers; ${this._cached ? this._cached.length : -1} bytes content`
        }
    }

    export interface RequestOptions {
        data?: string | Buffer;
        json?: any; // will call JSON.stringify()
        headers?: StringMap;
        stream?: boolean;
        timeout?: number; // in ms
    }

    function dataAsBuffer(data: string | Buffer): Buffer {
        if (data == null)
            return null
        if (typeof data == "string")
            return control.createBufferFromUTF8(data)
        return data
    }

    /*
    >>> "a,b,c,d,e".split(",", 2)
    ['a', 'b', 'c,d,e']
    */
    function pysplit(str: string, sep: string, limit: number) {
        const arr = str.split(sep)
        if (arr.length >= limit) {
            return arr.slice(0, limit).concat([arr.slice(limit).join(sep)])
        } else {
            return arr
        }
    }


    /** Perform an HTTP request to the given url which we will parse to determine
whether to use SSL ('https://') or not. We can also send some provided 'data'
or a json dictionary which we will stringify. 'headers' is optional HTTP headers
sent along. 'stream' will determine if we buffer everything, or whether to only
read only when requested
 
*/
    export function request(net: net.Net, method: string, url: string, options?: RequestOptions): net.Response {
        if (!options) options = {};
        if (!options.headers) {
            options.headers = {}
        }

        const tmp = pysplit(url, "/", 3)
        let proto = tmp[0]
        let host = tmp[2]
        let path = tmp[3] || ""
        // replace spaces in path
        // TODO
        // path = path.replace(" ", "%20")

        let port = 0
        if (proto == "http:") {
            port = 80
        } else if (proto == "https:") {
            port = 443
        } else {
            control.fail("Unsupported protocol: " + proto)
        }

        if (host.indexOf(":") >= 0) {
            const tmp = host.split(":")
            host = tmp[0]
            port = parseInt(tmp[1])
        }

        let ipaddr = net.hostbyName(host)

        let sock: Socket;
        const secure = ;
        if (proto == "https:") {
            // for SSL we need to know the host name
            sock = net.createSocket(host, port, true)
        } else {
            sock = net.createSocket(ipaddr, port, true)
        }
        // our response
        let resp = new Response(sock)
        // socket read timeout
        sock.setTimeout(options.timeout)

        sock.connect();
        sock.send(`${method} /${path} HTTP/1.0\r\n`)

        if (!options.headers["Host"])
            sock.send(`Host: ${host}\r\n`)

        if (!options.headers["User-Agent"])
            sock.send("User-Agent: MakeCode ESP32\r\n")

        // Iterate over keys to avoid tuple alloc
        for (let k of Object.keys(options.headers))
            sock.send(`${k}: ${options.headers[k]}\r\n`)

        if (options.json != null) {
            control.assert(options.data == null, 100)
            options.data = JSON.stringify(options.json)
            sock.send("Content-Type: application/json\r\n")
        }

        let dataBuf = dataAsBuffer(options.data)

        if (dataBuf)
            sock.send(`Content-Length: ${dataBuf.length}\r\n`)

        sock.send("\r\n")
        if (dataBuf)
            sock.send(dataBuf)

        let line = sock.readLine()
        // print(line)
        let line2 = pysplit(line, " ", 2)
        let status = parseInt(line2[1])
        let reason = ""
        if (line2.length > 2) {
            reason = line2[2]
        }

        while (true) {
            line = sock.readLine()
            if (!line || line == "\r\n") {
                break
            }

            // print("**line: ", line)
            const tmp = pysplit(line, ": ", 1)
            let title = tmp[0]
            let content = tmp[1]
            if (title && content) {
                resp.headers[title.toLowerCase()] = content.toLowerCase()
            }
        }

        /*
    
    elif line.startswith(b"Location:") and not 200 <= status <= 299:
    raise NotImplementedError("Redirects not yet supported")
    */

        if ((resp.headers["transfer-encoding"] || "").indexOf("chunked") >= 0)
            control.fail("not supported chunked encoding")

        resp.status_code = status
        resp.reason = reason
        return resp
    }

    /** Send HTTP HEAD request */
    export function head(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "HEAD", url, options)
    }

    /** Send HTTP GET request */
    export function get(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "GET", url, options)
    }

    /** Send HTTP POST request */
    export function post(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "POST", url, options)
    }

    /** Send HTTP PATCH request */
    export function patch(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "PATCH", url, options)
    }

    /** Send HTTP PUT request */
    export function put(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "PUT", url, options)
    }

    /** Send HTTP DELETE request */
    export function del(net: net.Net, url: string, options?: RequestOptions) {
        return request(net, "DELETE", url, options)
    }
}