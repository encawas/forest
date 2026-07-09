'use strict';

const { createApp } = require('./app');

const port = Number(process.env.PORT || 3001);
const host = process.env.HOST || '0.0.0.0';

const { app } = createApp();

app.listen(port, host, () => {
  console.log(`EchoForest backend listening on http://${host}:${port}`);
});
