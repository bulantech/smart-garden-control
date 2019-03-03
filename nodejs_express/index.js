const express = require('express')
const cors = require('cors')
const bodyParser = require("body-parser");
const app = express()

const port = 3000

app.use(cors())
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.get('/', (request, response) => {
  response.send('Hello from Express!')
})
app.get('/status', (request, response) => {
	let relay = Math.round(Math.random())
	let sensor = Math.floor(Math.random() * 1024);
  response.send(sensor + ',' + relay+ ',1000,300,30')
})
app.post('/save', (request, response) => {
	console.log(request.body)
	let startValue = request.body.startValue
  let stopValue = request.body.stopValue
  let maxTime = request.body.maxTime
  
  response.send('Save ok')
})
app.get('/on', (request, response) => {
  response.send('ON')
})
app.get('/off', (request, response) => {
  response.send('OFF')
})

app.listen(port, (err) => {
  if (err) {
    return console.log('something bad happened', err)
  }

  console.log(`server is listening on ${port}`)
})