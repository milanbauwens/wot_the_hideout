import express from "express";
import { toggleBulb, setBulb, initBuld, setBlackLight, disconnectBulb } from "../app.js";

const app = express()
const port = 3000

app.get('/', (req, res) => {
    const bulb = initBuld();
    res.send(bulb)
})

// Opposite of current state
app.post('/toggle', (req, res) => {
    toggleBulb();
    res.send('Bulb toggled');
})

// Turn bulb on
app.post('/on', (req, res) => {
    setBulb(true);
    res.send('Bulb on');
})

// turn bulb off
app.post('/off', (req, res) => {
    setBulb(false);
    res.send('Bulb off');
})

/* // Get blacklight scene
app.post('/blacklight', (req, res) => {
    console.log(req.body);

    res.send('Blacklight');
})
 */
app.post('/disconnect', (req, res) => {
    disconnectBulb();
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})