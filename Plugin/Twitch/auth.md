# Auth


- TS initiates a login request to https://battlechicken.hu
    - server returns random number
    - should have exiration date
- use random number to initiate browser login
    - validate random number ( nonce? )
    - send user to twitch (state = nonce)
    - get access token
    - save access token, tied to nonce
- ts uses to poll token by nonce

