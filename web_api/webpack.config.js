var path = require('path');
var webpack = require('webpack');

module.exports = {
  entry: './source/index.ts',
  output: {
    path: path.resolve(__dirname, 'library/beam'),
    filename: 'index.js',
    libraryTarget: 'umd',
    library: 'Beam',
    umdNamedDefine: true
  },
  resolve: {
    extensions: ['.ts', '.js']
  },
  devtool: 'source-map',
  module: {
    rules: [
      {
        test: /\.ts$/,
        loader: 'ts-loader'
      },
      {
        test: /\.js$/,
        loader: 'source-map-loader',
        enforce: 'pre'
      }
    ]
  }
};
