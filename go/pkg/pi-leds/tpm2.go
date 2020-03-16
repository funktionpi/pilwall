package pileds

import (
	"bytes"
	"errors"
	"fmt"
	"image"
	"net"
	"time"

	"github.com/draeron/gopkg/color"
	"github.com/funktionpi/pilwall/go/pkg/layout"
)

/*
  Specs: https://gist.github.com/jblang/89e24e2655be6c463c56
*/
type Tpm2 struct {
	udpsock net.Conn
	packetsPerFrame int
	mosaic layout.Mosaic
}

type tpm2Packet struct {
	typed byte
	index byte
	count byte
	data  []byte
}

const (
	TPM2NetMaxSize = 1490

	DefaultTpm2InPort  = 65506
	DefaultTpm2OutPort = 65442

	TPM2NetStart   = 0x9c
	TPM2NetData    = 0xda
	TPM2NetCommand = 0xc0
	TPM2NetRequest = 0xaa
	TPM2NetEnd     = 0x36
)

func ConnectTpm2(ip net.IP, mosaic layout.Mosaic) (Client, error) {
	c := &Tpm2{
		packetsPerFrame: 1,
		mosaic: mosaic,
	}

	conn, err := net.DialTimeout("udp", fmt.Sprintf("%s:%d", ip.String(), DefaultTpm2InPort), time.Second*3)
	if err != nil {
		return nil, err
	}
	c.udpsock = conn

	for int(mosaic.Count())/c.packetsPerFrame*3 > TPM2NetMaxSize {
		c.packetsPerFrame *= 2
	}

	return c, nil
}

func (t *Tpm2) readPacket() (*tpm2Packet, error) {

	buf := [1500]byte{} // maximum size for a udp packet

	size, err := t.udpsock.Read(buf[0:])
	if err != nil {
		return nil, err
	}

	// minimal size of a packet is 8 bytes
	if size < 8 {
		return nil, errors.New("tpm2 packet size is too small")
	}

	packet := tpm2Packet{}

	rdr := bytes.NewReader(buf[:])
	if start, _ := rdr.ReadByte(); start != TPM2NetStart {
		return nil, errors.New("tpm2 packet didn't start with proper marker")
	}

	switch val, _ := rdr.ReadByte(); val {
	case TPM2NetData:
	case TPM2NetCommand:
	case TPM2NetRequest:
	default:
		return nil, errors.New("unknown tpm2net message")
	}

	return &packet, nil
}

func (t *Tpm2) sendData(bytes []byte) error {

	bytesPerPacket := int(t.mosaic.Count()) / t.packetsPerFrame * 3

	for i := 0; i < t.packetsPerFrame; i++ {
		p := tpm2Packet{
			typed: TPM2NetData,
			index: byte(i+1),
			count: byte(t.packetsPerFrame), // packet are one-based
			data:  bytes[i*bytesPerPacket:i*bytesPerPacket+bytesPerPacket],
		}
		err := t.sendPacket(&p)
		if err != nil {
			return err
		}
	}

	return nil
}

func (t *Tpm2) sendPacket(packet *tpm2Packet) error {
	buf := bytes.NewBuffer([]byte{})

	buf.WriteByte(TPM2NetStart)
	buf.WriteByte(TPM2NetData)

	size := len(packet.data)

	if size > 1490 {
		return errors.New("packet data is too big")
	}

	buf.WriteByte(byte(size >> 8))
	buf.WriteByte(byte(size & 8))
	buf.WriteByte(packet.index)
	buf.WriteByte(packet.count)
	buf.Write(packet.data)
	buf.WriteByte(TPM2NetEnd)

	_, err := t.udpsock.Write(buf.Bytes())
	return err
}

func (t *Tpm2) Close() {
	if t.udpsock != nil {
		t.udpsock.Close()
		t.udpsock = nil
	}
}

// TODO: set brightness through a CMD packet
func (t *Tpm2) SetBrightness(brightness uint8) error {
	//return errors.New("not supported")
	return nil
}

// TODO: clear color through a CMD packet
func (t *Tpm2) Clear(color color.Color) error {
	rgb := color.RGB()
	return t.sendPacket(&tpm2Packet{
		typed: TPM2NetCommand,
		count: 1,
		index: 1,
		data: []byte{rgb.R, rgb.G, rgb.B},
	})
}

func (t *Tpm2) UpdateScreen() error {
	return nil
}

func (t *Tpm2) DrawImgRaw(img image.Image, mosaic layout.Mosaic) error {
	bytes := mosaic.SliceToBytes(img)
	return t.sendData(bytes)
}

func (t *Tpm2) DrawImg(img image.Image) error {
	bytes := t.mosaic.SliceToBytes(img)
	return t.sendData(bytes)
}
