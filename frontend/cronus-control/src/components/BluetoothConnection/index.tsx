import * as React from "react";
import {Component} from "react";
import {useTranslation} from 'react-i18next';
import Button from '@mui/material/Button';
import BluetoothIcon from '@mui/icons-material/Bluetooth';

type BluetoothConnectionProps = {
    onBluetoothConnected: () => void;
    onWiFiListUpdate: (list: Array<string>) => void;
}

type BluetoothConnectionState = {
    connecting: boolean;
    connected: boolean;
}

export default class Connection extends Component<BluetoothConnectionProps, BluetoothConnectionState> {
    btDev: BluetoothDevice | undefined;
    btSrv: BluetoothRemoteGATTServer | undefined;
    btSvc: BluetoothRemoteGATTService | undefined;
    btChc: BluetoothRemoteGATTCharacteristic | undefined;

    constructor() {
        super();

        this.state = {
            connecting: false,
            connected: false,
        };

        setInterval(this.scanWiFi.bind(this), 10000);
        setInterval(this.readWiFiList.bind(this), 5000);
    }

    async connect() {
        this.setState({connecting: true});

        if (!(await navigator.bluetooth.getAvailability())) {
            throw ("bluetooth is not available");
        }

        try {
            this.btDev = await navigator.bluetooth.requestDevice({
                filters: [
                    {services: [0xFFFF]},
                    {namePrefix: "Cronus"}
                ],
                optionalServices: [0xFFFF],
            });

            this.btSrv = await this.btDev.gatt?.connect();
            this.btSvc = await this.btSrv?.getPrimaryService(0xFFFF);
            this.btChc = await this.btSvc?.getCharacteristic(0xFF01);
        } catch (e) {
            this.setState({connecting: false});
            throw e;
        }

        this.setState({
            connecting: false,
            connected: true,
        });

        this.props.onBluetoothConnected();
    }

    async scanWiFi() {
        if (!this.state.connected) {
            return;
        }

        if (!this.btChc) {
            throw "The device is not connected";
        }

        const val = new Int8Array(97);
        val.set([1], 0);
        await this.btChc.writeValueWithoutResponse(val);

        console.log(this);
    }

    readWiFiList() {
        if (!this.state.connected) {
            return;
        }

        const dec = new TextDecoder("utf-8");
        const newList = [];

        this.btChc.readValue().then(val => {
            const state = val.getInt8(1);
            console.log("state", state);

            let offset = 33;
            for (let i = 0; i < 5; i++) {
                const ssid = dec.decode(val.buffer.slice(offset, offset + 32));
                offset += 32;

                if (ssid == "") {
                    continue;
                }

                newList.push(ssid);
            }

            // this.setState({ssids: newList});
            this.props.onWiFiListUpdate(newList);
        });
    }

    render() {
        const {t} = useTranslation();

        return (
            <Button
                variant={"outlined"}
                onClick={_ => this.connect()}
                disabled={this.state.connecting || this.state.connected}
                sx={({display: this.state.connected ? "none" : "inline-flex"})}
            >
                <BluetoothIcon></BluetoothIcon>
                {this.state.connecting ? t("connecting") + "..." : t("connect")}
            </Button>
        );
    }
}
