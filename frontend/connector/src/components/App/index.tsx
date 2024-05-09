import * as React from 'react';
import CssBaseline from '@mui/material/CssBaseline';
import Grid from '@mui/material/Grid';

import * as BT from "../Bluetooth";
import * as WiFi from "../WiFi";
import {Component} from "react";

interface Props {
}

interface State {
    btConnStatus: BT.ConnStatus;
    wifiNextScan: number;
    wifiConnStatus: WiFi.ConnStatus;
    wifiLastError: WiFi.ErrorReason;
    wifiSSIDList: Array<string>;
    wifiConnectedSSID: string;
}

export default class App extends Component <Props, State> {
    readBTChc: (uuid: number) => Promise<DataView>;
    writeBTChc: (uuid: number, data: BufferSource) => void;

    constructor() {
        super();

        this.state = {
            btConnStatus: BT.ConnStatus.DISCONNECTED,
            wifiNextScan: 0,
            wifiConnStatus: WiFi.ConnStatus.DISCONNECTED,
            wifiLastError: WiFi.ErrorReason.NO,
            wifiSSIDList: [],
            wifiConnectedSSID: "",
        };

        setInterval(this.fetchWiFiStatus.bind(this), 1000);
    }

    trimByteStr(s: string): string {
        let endPos = s.length - 1;
        for (let j = 0; j < s.length; j++) {
            if (s.codePointAt(j) == 0) {
                endPos = j;
                break;
            }
        }

        return s.substring(0, endPos);
    }

    async fetchWiFiStatus(): Promise<void> {
        if (this.state.btConnStatus != BT.ConnStatus.CONNECTED) {
            return;
        }

        // byte 0:       state: disconnected(0), scanning(1), connecting(2), connected(3), error(4)
        // byte 1-32:    connected SSID or error description
        // byte 33-64:   scanned SSID 1
        // byte 65-96:   scanned SSID 2
        // byte 97-128:  scanned SSID 3
        // byte 129-160: scanned SSID 4
        // byte 161-192: scanned SSID 5
        const data = await this.readBTChc(BT.ChcUUID.WIFI);

        const dec = new TextDecoder("utf-8");
        const newList = [];

        const d = data.getInt8(0);
        const st: WiFi.ConnStatus = d & 0x0f; // low 4 bits
        const errReason = d >> 4; // high 4 bits

        this.setState({
            wifiConnStatus: st,
            wifiLastError: errReason,
        });

        switch (st) {
            case WiFi.ConnStatus.DISCONNECTED:
                let offset = 33;
                for (let i = 0; i < 5; i++) {
                    const ssid = this.trimByteStr(dec.decode(data.buffer.slice(offset, offset + 32)));
                    if (ssid != "") {
                        newList.push(ssid);
                    }
                    offset += 32;
                }

                this.setState({
                    wifiSSIDList: newList,
                });

                if (this.state.wifiConnStatus != WiFi.ConnStatus.CONNECTED &&
                    this.state.wifiConnStatus != WiFi.ConnStatus.CONNECTING &&
                    Date.now() >= this.state.wifiNextScan) {
                    this.requestWiFiScan();
                    this.setState({wifiNextScan: Date.now() + 10000});
                }

                break;

            case WiFi.ConnStatus.CONNECTED:
                this.setState({
                    wifiConnectedSSID: this.trimByteStr(dec.decode(data.buffer.slice(1, 33))),
                });

                break;
        }
    }

    requestWiFiScan(): void {
        const val = new Int8Array(1);
        val.set([WiFi.Op.SCAN], 0);
        this.writeBTChc(BT.ChcUUID.WIFI, val);
    }

    requestWiFiConnect(c: WiFi.Credentials) {
        this.setState({wifiConnStatus: WiFi.ConnStatus.CONNECTING});

        const enc = new TextEncoder();

        const val = new Int8Array(97);
        val.set([WiFi.Op.CONNECT], 0);
        val.set(enc.encode(c.ssid), 1)
        val.set(enc.encode(c.password), 33)

        this.writeBTChc(BT.ChcUUID.WIFI, val);
    }

    requestWiFiDisconnect() {
        this.setState({
            wifiConnectedSSID: "",
        });

        const val = new Int8Array(1);
        val.set([WiFi.Op.DISCONNECT], 0);
        this.writeBTChc(BT.ChcUUID.WIFI, val);
    }

    render() {
        return (
            <Grid container
                  spacing={0}
                  direction="column"
                  alignItems="center"
                  justifyContent="center"
                  sx={{minHeight: '100vh'}}
            >
                <CssBaseline/>

                <BT.Connection
                    onStatusChange={s => {
                        this.setState({btConnStatus: s});
                        if (s == BT.ConnStatus.DISCONNECTED) {
                            this.setState({
                                wifiConnStatus: WiFi.ConnStatus.DISCONNECTED,
                                wifiSSIDList: [],
                                wifiConnectedSSID: "",
                            });
                        }
                    }}
                    onSetChcReader={f => (this.readBTChc = f)}
                    onSetChcWriter={f => (this.writeBTChc = f)}
                />

                <WiFi.Connection
                    visible={this.state.btConnStatus == BT.ConnStatus.CONNECTED}
                    connStatus={this.state.wifiConnStatus}
                    SSIDs={this.state.wifiSSIDList}
                    connectedSSID={this.state.wifiConnectedSSID}
                    onConnectRequest={this.requestWiFiConnect.bind(this)}
                    onDisconnectRequest={this.requestWiFiDisconnect.bind(this)}
                />
            </Grid>
        );
    }
}