package slave

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
	"image/color"
	"led-wall/pkg/errors"
	"net/url"
	"time"
)

//go:generate protoc -I ../../../slave/lib/proto -I $NANOPB/generator/proto --go_out=. ledctrl.proto

type Client struct {
	conn *websocket.Conn

	responses map[int32]*chan *Response
	next int32;
}

func Connect(url url.URL) (*Client, error) {

	fmt.Printf("connecting to ledslave '%s'\n", url.String())
	conn, resp, err := websocket.DefaultDialer.Dial(url.String(), nil)
	if err != nil {
		return nil, err
	}

	fmt.Printf("websocket response: %v\n", resp.Status)

	channelMap := make(map[int32]*chan *Response)

	cl := &Client{
		conn: conn,
		responses: channelMap,
		next: 1,
	}

	go cl.processResponse()

	return cl, nil
}

func (c *Client) Close() {
	err := c.conn.Close()
	errors.PrintIfErr(err)
}

func (c *Client) processResponse() {
	for {
		typed, buf, err := c.conn.ReadMessage()

		if err != nil {
			errors.PrintIfErr(err)
			continue
		}

		if typed == websocket.BinaryMessage {
			resp := Response{}
			err = proto.Unmarshal(buf, &resp)
			if err != nil {
				errors.PrintIfErr(err)
			} else {
				if ch, ok := c.responses[resp.Id]; ok {
					*ch <- &resp
					delete(c.responses, resp.Id)
				} else {
					fmt.Printf("could not find channel for request %d\n", resp.Id)
				}
			}
		} else {
			fmt.Println("received text message: ", string(buf))
		}
	}
}

func (c *Client) SetBrightness(brightness uint8) error {

	fmt.Printf("setting brightness to %d\n", brightness)

	req := &Request_Brightness{&BrightnessRequest{
			Brightness: int32(brightness),
		},
	}

	_, err := c.send(req)
	return err
}

func (c *Client) Clear(color color.RGBA) error {
  val := convertColor(color)
	fmt.Printf("Clear color: %v\n", color)

	req := &Request_Clear{		&ClearRequest{
			Color: val,
		},
	}
	_, err := c.send(req)
	return err
}

func (c *Client) GetDimension() (*DimensionResponse, error) {
	req := &Request_Dimension{&DimensionRequest{}}
	msg, err := c.send(req)
	if err != nil {
		return nil, err
	}

	pkg, ok := msg.Response.(*Response_Dimension)
	if !ok {
		return nil, fmt.Errorf("wrong resonse datatype")
	}

	return pkg.Dimension, nil
}

func (c *Client) UpdateScreen() {
}

func (c *Client) DrawLine(x1,y1,x2,y2 int16, rgba color.RGBA) error {
	col := convertColor(rgba)
	fmt.Printf("Line from (%d,%d) to (%d,%d) => Color: %v\n", x1,y1,x2,y2,rgba)

	req := &Request_DrawLine{&DrawLineRequest{
		Color:                col,
		Start:                convertCoord(x1,y1),
		End:                  convertCoord(x2,y2),
	}}

	_, err := c.send(req)
	return err
}

func (c *Client) send(req isRequest_Request) (*Response, error) {

	msg := Request{
		Id: c.next,
		Request: req,
	}

	respChan := make(chan *Response)
	c.responses[c.next] = &respChan
	c.next++


	buf, err := proto.Marshal(&msg)
	if err != nil {
		return nil, err
	}

	err = c.conn.WriteMessage(websocket.BinaryMessage, buf)
	if err != nil {
	  return nil, err
  }

	// block until we received a response
	select {
	case response := <- respChan:
		//println("response received successfully")
		return response, nil
	case <- time.After(1 * time.Second):
		println("response timeout")
		return nil, fmt.Errorf("response timeout")
	}
}

// Downgrade 24-bit color to 16-bit (add reverse gamma lookup here?)
func convertColor(color color.RGBA) uint32 {
  return (uint32(color.R) << 16) | (uint32(color.G) << 8) | (uint32(color.B))
}

func convertCoord(x,y int16) *Coord {
	return &Coord{
		Xy: ((int32(x) << 16) | int32(y)),
	}
}
