const express = require('express')
const cors = require('cors')
const app = express()
const port = 3000

app.use(cors())

app.get('/', (request, response) => {
  response.send('Hello from Express!')
})
app.get('/status', (request, response) => {
	let relay = Math.round(Math.random())
	let sensor = Math.floor(Math.random() * 1024);
  response.send(sensor + ',' + relay+ ',1000,300,30')
})

app.listen(port, (err) => {
  if (err) {
    return console.log('something bad happened', err)
  }

  console.log(`server is listening on ${port}`)
})