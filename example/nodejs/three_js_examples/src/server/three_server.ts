import express from 'express'
import path from 'path'
import http from 'http'

const port: number = 3000
const hostname:string = '127.0.0.1'

class App {
    private server: http.Server
    private port: number

    constructor(port: number) {
        this.port = port
        const app = express()
        app.use(express.static(path.join(__dirname, '../client')))
        this.server = new http.Server(app)
    }

    public Start() {
        this.server.listen(this.port, () => {
            console.log(`Server running at http://${hostname}:${port}/`)
        })
    }
}

new App(port).Start()