import * as React from "react";
import {useTranslation} from 'react-i18next';
import Button from '@mui/material/Button';
import BluetoothIcon from '@mui/icons-material/Bluetooth';

export enum ConnStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
}

export enum ChcUUID {
    WIFI = 0xff01,
}

const btServiceUUID = 0xffff;

interface Props {
    onStatusChange?: (s: ConnStatus) => void;
    onSetChcReader?: (f: (uuid: number) => Promise<DataView>) => void;
    onSetChcWriter?: (f: (uuid: number, data: DataView) => void) => void;
}

type State = {
    connStatus: ConnStatus;
};

export class Connection extends React.Component<Props, State> {
    connStatus: ConnStatus;

    btDev?: BluetoothDevice;
    btSrv?: BluetoothRemoteGATTServer;
    btSvc?: BluetoothRemoteGATTService;
    btChrcs: { [uuid: number]: BluetoothRemoteGATTCharacteristic }

    constructor() {
        super();

        this.state = {
            connStatus: ConnStatus.DISCONNECTED,
        };

        this.btChrcs = {};

        setInterval(this.connStatusChecker.bind(this), 500);
    }

    connStatusChecker():void {
        if (!(this.btSrv && this.btSrv.connected) && this.connStatus !== ConnStatus.CONNECTING) {
            this.setConnStatus(ConnStatus.DISCONNECTED);
        }
    }

    setConnStatus(s: ConnStatus): void {
        if (s == this.connStatus) {
            return;
        }

        this.connStatus = s
        this.setState({connStatus: s});

        if (this.props.onStatusChange) {
            this.props.onStatusChange(s);
        }
    }

    componentDidMount(): void {
        if (this.props.onSetChcReader) {
            this.props.onSetChcReader(this.chcReader.bind(this));
        }

        if (this.props.onSetChcWriter) {
            this.props.onSetChcWriter(this.chcWriter.bind(this));
        }
    }

    async chcReader(uuid: number): Promise<DataView> {
        if (!this.btSrv || !this.btSrv.connected) {
            if (this.props.onStatusChange) {
                this.props.onStatusChange(ConnStatus.DISCONNECTED);
            }
            throw "not connected"
        }

        if (!this.btChrcs[uuid]) {
            throw "unsupported characteristic: " + uuid
        }

        return this.btChrcs[uuid].readValue();
    }

    async chcWriter(uuid: number, data: BufferSource): Promise<void> {
        if (!this.btDev || !this.btSrv.connected) {
            if (this.props.onStatusChange) {
                this.props.onStatusChange(ConnStatus.DISCONNECTED);
            }
            throw "not connected"
        }

        if (!this.btChrcs[uuid]) {
            throw "unsupported characteristic: " + uuid
        }

        return this.btChrcs[uuid].writeValueWithoutResponse(data);
    }

    async connect() {
        if (!(await navigator.bluetooth.getAvailability())) {
            throw ("bluetooth is not available");
        }

        this.setConnStatus(ConnStatus.CONNECTING);

        try {
            this.btDev = await navigator.bluetooth.requestDevice({
                filters: [
                    {services: [btServiceUUID]},
                    {namePrefix: "D5Y"},
                ],
            });

            this.btSrv = await this.btDev.gatt?.connect();
            this.btSvc = await this.btSrv?.getPrimaryService(btServiceUUID);
            this.btChrcs[ChcUUID.WIFI] = await this.btSvc?.getCharacteristic(ChcUUID.WIFI);

            this.setConnStatus(ConnStatus.CONNECTED);
        } catch (e) {
            this.setConnStatus(ConnStatus.DISCONNECTED);
            throw e;
        }
    }

    render() {
        if (this.state.connStatus  == ConnStatus.CONNECTED) {
            return <React.Fragment/>
        }

        const {t} = useTranslation();

        return (
            <Button
                variant={"contained"}
                onClick={this.connect.bind(this)}
                disabled={this.state.connStatus == ConnStatus.CONNECTING}
            >
                <BluetoothIcon/>
                {this.state.connStatus == ConnStatus.CONNECTING ? t("connecting") + "..." : t("connect")}
            </Button>
        );
    }
}
