package pileds

import "net"

/*
  Specs: https://gist.github.com/jblang/89e24e2655be6c463c56
*/
type Tpm2Client struct {
	udpsock net.Conn
}
