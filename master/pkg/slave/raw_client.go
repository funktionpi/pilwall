package slave

import (
  "bytes"
  "encoding/binary"
  "fmt"
  "github.com/golang/protobuf/proto"
  "image"
  "led-wall/pkg/chrono"
  "led-wall/pkg/color"
  "led-wall/pkg/errors"
  "led-wall/pkg/layout"
  "net"
  "net/url"
  "time"
)

type RawClient struct {
  rawsock net.Conn
  tcpsock net.Conn
  lastImg image.Image
  next    int32
}

type udpPacket struct {
  index  uint16
  count  uint8
  colors []uint8
}

func ConnectUdp(url url.URL) (*RawClient, error) {
  c := &RawClient{
    next: 1,
  }
  fmt.Printf("connecting to ledslave '%s'\n", url.String())
  var err error

  c.rawsock, err = net.DialTimeout("udp", url.Host + ":3001", time.Second*3)
  if err != nil {
    return nil, err
  }

  c.tcpsock, err = net.DialTimeout("udp", url.Host + ":4001", time.Second*3)
  if err != nil {
    return nil, err
  }

  // wait a little before sending stuff over the wire
  time.Sleep(time.Millisecond * 500)

  return c, nil
}

func (c *RawClient) Close() {
  var err error
  if c.rawsock != nil {
    err = c.rawsock.Close()
  }
  errors.PrintIfErr(err)

  if c.tcpsock != nil {
    err = c.tcpsock.Close()
  }
  errors.PrintIfErr(err)
}

func (c *RawClient) SetBrightness(brightness uint8) error {
  fmt.Printf("setting brightness to %d\n", brightness)

  req := &Request_Brightness{&BrightnessRequest{
    Brightness: int32(brightness),
  }}

  _, err := c.sendTcp(req)
  return err
}

func (c *RawClient) Clear(color color.Color) error {
  val := convertColor(color)
  fmt.Printf("Clear color: %v\n", color)

  req := &Request_Clear{&ClearRequest{
    Color: val,
  }}
  _, err := c.sendTcp(req)
  return err
}

func (c *RawClient) GetDimension() (*DimensionResponse, error) {
  req := &Request_Dimension{&DimensionRequest{}}
  msg, err := c.sendTcp(req)
  if err != nil {
    return nil, err
  }

  pkg, ok := msg.Response.(*Response_Dimension)
  if !ok {
    return nil, fmt.Errorf("wrong resonse datatype")
  }

  return pkg.Dimension, nil
}

func (c *RawClient) UpdateScreen() error {
  req := &Request_Update{
    Update: &UpdateRequest{},
  }

  _, err := c.sendTcp(req)
  return err
}

func (c *RawClient) DrawLine(x1, y1, x2, y2 int16, rgba color.Color) error {
  col := convertColor(rgba)
  fmt.Printf("Line from (%d,%d) to (%d,%d) => Color: %v\n", x1, y1, x2, y2, rgba)

  req := &Request_DrawLine{&DrawLineRequest{
    Color: col,
    Start: convertCoord(x1, y1),
    End:   convertCoord(x2, y2),
  }}

  _, err := c.sendTcp(req)
  return err
}

func (c *RawClient) DrawImgRaw(img image.Image, mosaic layout.Mosaic) error {
  raw := mosaic.SliceToBytes(img)
  index := uint16(0)

  sw := chrono.NewStopWatch(false)

  maxLength := 64 * 3

  for len(raw) > 0 {
    buf := &bytes.Buffer{}
    _ = binary.Write(buf, binary.LittleEndian, index)

    count := uint8(0)
    if len(raw) > maxLength {
      count = uint8(maxLength / 3)
    } else {
      count = uint8(len(raw) / 3)
    }
    buf.WriteByte(count)
    index += uint16(count)
    buf.Write(raw[:count*3])
    raw = raw[count*3:]

    sw.Start()
    _, err := c.rawsock.Write(buf.Bytes())
    errors.PrintIfErr(err)
    sw.Stop()
  }

  qwe := sw.Elapsed().Milliseconds()
  println("elapsed: ", qwe, "ms")

  return nil
}

func (c *RawClient) sendTcp(req isRequest_Request) (*Response, error) {

  msg := Request{
    Id:      c.next,
    Request: req,
  }

  c.next++

  buf, err := proto.Marshal(&msg)
  if err != nil {
    return nil, err
  }

  _, err = c.tcpsock.Write(buf)
  if err != nil {
    return nil, err
  }

  //err = c.tcpsock.SetReadDeadline(time.Now().Add(time.Second))
  //if err != nil {
  //  return nil, err
  //}

  buf = []byte{}
  _, err = c.tcpsock.Read(buf)
  if err != nil {
    return nil, err
  }

  resp := Response{}
  err = proto.Unmarshal(buf, &resp)
  if err != nil {
    return nil, err
  }

  if resp.Id != msg.Id {
    return nil, fmt.Errorf("received response with id %d, was expecting %d", resp.Id, msg.Id)
  }

  return &resp, nil
}
