import * as mqtt from "mqtt"  // import everything inside the mqtt module and give it the namespace "mqtt"
import express from "express";
import { Server } from "socket.io";
import http from "http";
import path from "path";

/* MQTT SERVER CONST */
const host = '192.168.50.101' // IP address of the MQTT server
const mqtt_port = '1883' // Port of the MQTT server
const clientId = `mqtt_${Math.random().toString(16).slice(3)}` // Generate a random client ID

const connectUrl = `mqtt://${host}:${mqtt_port}`

const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: 'admin',
  password: '1234',
  reconnectPeriod: 1000,
})

const publishTopic = 'Room/My room/Props/CRT/outbox'
const subscribeTopic = 'Room/My room/Props/CRT/inbox'

const app = express()
const server = http.createServer(app)
const port = 3000
const __dirname = process.cwd()
const io = new Server(server)


client.on('connect', () => {
  client.subscribe([subscribeTopic], () => {
    console.log(`Subscribe to topic '${subscribeTopic}'`)
  })

  app.use(express.static(path.join(__dirname, 'media')));

  app.get('/', (req, res) => {
    res.sendFile('index.html', { root: __dirname })
  })

  io.on('connection', (socket) => {
    console.log('a user connected');
    socket.on('disconnect', () => {
        console.log('user disconnected');
    });
  })

  server.listen(port, () => {
    console.log(`Example app listening on port ${port}`)
  })
})

client.on('message', (publishTopic, payload) => {
    const message = payload.toString();
    io.emit('message', message);
})

client.on('connect', () => {
    client.publish(publishTopic, 'nodejs mqtt test', { qos: 0, retain: false }, (error) => {
      if (error) {
          console.error(error)
      }
    })
})