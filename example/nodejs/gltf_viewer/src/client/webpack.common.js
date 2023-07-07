const path = require('path');

module.exports = {
    entry: './src/client/threeClient.ts',
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            },
            {
                test: /\.(png|svg|jpg|jpeg|gif|envmap)$/i,
                type: 'asset/resource',
            },
            {
                test: /\.(glsl|vs|fs|vert|frag)$/,
                use: [
                  'raw-loader',
                ]
            }
        ],
    },
    resolve: {
        alias: {
            three: path.resolve('./node_modules/three'),
            '../ao/AOPass': path.resolve('./node_modules/realism-effects/src/ao/AOPass.js'),
            './utils/PoissonUtils': path.resolve('./node_modules/realism-effects/src/poissionDenoise/utils/PoissonUtils.js'),
        },
        extensions: ['.tsx', '.ts', '.js'],
    },
    output: {
        filename: 'bundle.js',
        path: path.resolve(__dirname, '../../dist/client'),
    }
};