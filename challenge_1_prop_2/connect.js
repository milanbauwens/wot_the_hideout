import * as mqtt from "mqtt"; // import everything inside the mqtt module and give it the namespace "mqtt"
import express from "express";
import {
  toggleBulb,
  setBulb,
  initBuld,
  setBlackLight,
  disconnectBulb,
} from "./app.js";
import fetch from "node-fetch";

/* MQTT SERVER CONST */
const host = "192.168.50.101"; // IP address   of the MQTT server
const mqtt_port = "1883"; // Port of the MQTT server
const clientId = `mqtt_${Math.random().toString(16).slice(3)}`; // Generate a random client ID

const connectUrl = `mqtt://${host}:${mqtt_port}`;

const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
  connectTimeout: 4000,
  username: "admin",
  password: "1234",
  reconnectPeriod: 1000,
});

const publishTopic = "Room/My room/Props/lamp/outbox";
const subscribeTopic = "Room/My room/Props/lamp/inbox";

client.on("connect", () => {
  client.subscribe([subscribeTopic], () => {
    console.log(`Subscribe to topic '${subscribeTopic}'`);
  });
});

client.on("message", (publishTopic, payload) => {
  const message = payload.toString();

  if (message === "1") {
    // Turn bulb on
    setBulb(true);

    // fetch("http://localhost:3000/on", {
    //   method: "POST",
    // });

    // client.publish(publishTopic, '1')
  } else if (message === "0") {
    // turn bulb off
    setBulb(false)
    // fetch("http://localhost:3000/off", {
    //   method: "POST",
    // });
    // client.publish(publishTopic, '0')
  } else {
    return;
  }
});

client.on("connect", () => {
  client.publish(
    publishTopic,
    "nodejs mqtt test",
    { qos: 0, retain: false },
    (error) => {
      if (error) {
        console.error(error);
      }
    }
  );
});

process.on('uncaughtException', function (err) {
  console.log(err);
});
