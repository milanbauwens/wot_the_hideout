import * as mqtt from "mqtt"; // import everything inside the mqtt module and give it the namespace "mqtt"
import express from "express";
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
  
    fetch("http://192.168.50.112/relay/0?turn=on", {
      method: "GET",
    });
  } else if (message === "0") {
    // turn bulb off
    fetch("http://192.168.50.112/relay/0?turn=off", {
      method: "GET",
    });
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
