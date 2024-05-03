import '@fontsource/roboto';
import {render} from 'preact';
// import './style.css';
import './i18n';

import * as React from 'react';
import CssBaseline from '@mui/material/CssBaseline';
import Grid from '@mui/material/Grid';
import Stack from '@mui/material/Stack';

import BluetoothConnection from "./components/BluetoothConnection";
import WiFiList from "./components/WiFiList";
import {Component} from "react";

type AppProps = {};

type AppState = {
    btConnected: boolean;
    wifiList: Array<string>;
};

export default class App extends Component <AppProps, AppState> {
    constructor() {
        super();

        this.state = {
            btConnected: false,
            wifiList: [],
        };
    }

    render() {
        return (
            <Grid
                container
                spacing={0}
                direction="column"
                alignItems="center"
                justifyContent="center"
                sx={{minHeight: '100vh'}}
            >
                <CssBaseline/>
                <Grid item xs={3}>
                    <Stack spacing={2} direction={"column"}>
                        <BluetoothConnection
                            onBluetoothConnected={() => this.setState({btConnected: true})}
                            onWiFiListUpdate={list => this.setState({wifiList: list})}
                        />
                        <WiFiList
                            hidden={!this.state.btConnected}
                            list={this.state.wifiList}
                            onItemClick={ssid => console.log(ssid)}
                        />
                    </Stack>
                </Grid>
            </Grid>
        );
    }
}

render(<App/>, document.getElementById('app'));
