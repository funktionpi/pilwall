package slave

import (
	"fmt"
	"image/color"
	"led-wall/pkg/errors"
	"net/url"

	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
)

//go:generate protoc -I ../../../slave/lib/proto -I $NANOPB/generator/proto --go_out=. ledctrl.proto

type Client struct {
	conn *websocket.Conn
}

func Connect(url url.URL) (*Client, error) {

	fmt.Printf("connecting to ledslave '%s'\n", url.String())
	conn, resp, err := websocket.DefaultDialer.Dial(url.String(), nil)
	if err != nil {
		return nil, err
	}

	fmt.Printf("websocket response: %v\n", resp.Status)

	return &Client{
		conn: conn,
	}, nil
}

func (c *Client) Close() {
	err := c.conn.Close()
	errors.PrintIfErr(err)
}

func (c *Client) SetBrightness(brightness uint8) error {

	req := &Request_BrightnessRequest{
		BrightnessRequest: &BrightnessRequest{
			Brightness: int32(brightness),
		},
	}

	return c.send(req)
}

func (c *Client) Clear(color color.RGBA) error {

  val := convertColor(color)
	fmt.Printf("Color: 0x%06x\n", val)

	req := &Request_ClearRequest{
		ClearRequest: &ClearRequest{
			Color: val,
		},
	}

	return c.send(req)
}

func (c *Client) send(req isRequest_Request) error {

	msg := Request{
		Request: req,
	}

	buf, err := proto.Marshal(&msg)
	if err != nil {
		return err
	}

	err = c.conn.WriteMessage(websocket.BinaryMessage, buf)
	if err != nil {
	  return err
  }

	// block until we received a response
	//msgtype, buf, err := c.conn.ReadMessage()
	//if err != nil {
	//  return err
  //}
  //
	//if msgtype != websocket.BinaryMessage {
	//  return fmt.Errorf("should have received a binary message")
  //}
  //
	//resp := &EmptyResponse{}
	//err = proto.Unmarshal(buf, resp)
	return err
}

// Downgrade 24-bit color to 16-bit (add reverse gamma lookup here?)
func convertColor(color color.RGBA) uint32 {
  return (uint32(color.R) << 16) | (uint32(color.G) << 8) | (uint32(color.B))
}