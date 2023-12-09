const path = require('path')
const { merge } = require('webpack-merge')
const commonConfiguration = require('./webpack.common.js')
const portFinderSync = require('portfinder-sync')

module.exports = merge(
    commonConfiguration,
    {
        stats: 'errors-warnings',
        mode: 'development',
        devServer:
        {
            host: 'local-ip',
            port: portFinderSync.getPort(8080),
            open: true,
            https: false,
            allowedHosts: 'all',
            hot: false,
            watchFiles: ['src/**', 'static/**'],
            static:
            {
                watch: true,
                directory: path.join(__dirname, '../static')
            },
            client:
            {
                logging: 'none',
                overlay: true,
                progress: false
            }
        }
    }
)
