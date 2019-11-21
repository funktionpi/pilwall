package slave

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
	"image/color"
	"led-wall/pkg/errors"
	"net"
	"net/url"
	"time"
)

//go:generate protoc -I ../../../slave/lib/proto -I $NANOPB/generator/proto --go_out=. ledctrl.proto

type Client struct {
	websock *websocket.Conn
	udpsock net.Conn

	responses map[int32]*chan *Response
	next int32;
}

func Connect(url url.URL) (*Client, error) {

	cl := &Client{
		responses: make(map[int32]*chan *Response),
		next:      1,
	}

	fmt.Printf("connecting to ledslave '%s'\n", url.String())
	if url.Scheme == "ws" {
		conn, resp, err := websocket.DefaultDialer.Dial(url.String(), nil)
		if err != nil {
			return nil, err
		}
		fmt.Printf("websocket response: %v\n", resp.Status)
		cl.websock = conn
	} else {
		conn, err := net.DialTimeout("udp", url.Host, time.Second * 3)
		if err != nil {
			return nil, err
		}
		cl.udpsock = conn
	}

	go cl.processResponse()
	return cl, nil
}

func (c *Client) Close() {
	var err error
	if err != nil {
		err = c.websock.Close()
	}
	errors.PrintIfErr(err)

	if c.udpsock != nil {
		err = c.udpsock.Close()
	}
	errors.PrintIfErr(err)
}

func (c *Client) processResponse() {
	for {
		var err error
		buf := []byte{}

		if c.websock != nil {
			var typed int
			typed, buf, err = c.websock.ReadMessage()
			if typed != websocket.BinaryMessage {
				fmt.Println("received text message: ", string(buf))
			}
		} else if c.udpsock != nil {
			tmp := [128]byte{}
			size := 0
			size, err = c.udpsock.Read(tmp[0:])
			buf = tmp[0:size]
		}

		if err != nil {
			errors.PrintIfErr(err)
			continue
		}

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

func (c *Client) UpdateScreen() error {
	req := &Request_Update{
		Update: &UpdateRequest{},
	}
	_, err := c.send(req)
	return err
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

	if c.websock != nil {
		err = c.websock.WriteMessage(websocket.BinaryMessage, buf)
	} else if c.udpsock != nil {
		_, err = c.udpsock.Write(buf)
	}
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
