### weekly update - casa0014 vespera

**goals & progress (from week 4)**

- Create fritzing diagram
Goal Achieved? Y
Implementation: Created a quick fritzing diagram to represent components needed and wiring (but not final orientation and building with enclosure)

- Fix wiring errors from previous week and ensure full electrical correctness
Goal Achieved? Y
Implementation: Swapped 1 faulty jumper wire & 1 misshapen button leg 

- Integrate mqtt 216 byte payload sending via wifi
Goal Achieved? Y
Implementation: Defined the function publishVesperaPayload, which uses mqttClient.publish from the PubSubClient library to send to mqtt broker 
