import * as React from "react";
import Stack from "@mui/material/Stack";
import Box from "@mui/material/Box";
import Button from "@mui/material/Button";
import Typography from "@mui/material/Typography";
import TextField from "@mui/material/TextField";
import WiFiIcon from "@mui/icons-material/Wifi";
import {ArrowBack} from "@mui/icons-material";
import CircularProgress from '@mui/material/CircularProgress';

export enum Op {
    NOP,
    SCAN,
    CONNECT,
    DISCONNECT,
}

export enum ConnStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
}

export enum ErrorReason {
    NO,
    UNKNOWN = 16,
}

export interface Credentials {
    ssid: string;
    password: string;
}

interface Props {
    visible: boolean;
    connStatus: ConnStatus;
    SSIDs: Array<string>;
    connectedSSID: string;
    onConnectRequest: (c: Credentials) => void;
    onDisconnectRequest: () => void;
}

interface State {
    credentials: Credentials;
    connTry: number;
}

export class Connection extends React.Component<Props, State> {
    constructor() {
        super();

        this.state = {
            credentials: {
                ssid: "",
                password: "",
            },
            connTry: 0,
        }
    }

    renderSSIDChooser(): React.JSX.Element {
        return (
            <Box sx={{flexDirection: "column", alignItems: "center"}}>
                <Box sx={{mb: 2}}>
                    Choose your Wi-Fi network:
                </Box>

                {!this.props.SSIDs.length &&
                    <Box sx={{textAlign: "center"}}>
                        <CircularProgress/>
                    </Box>
                }

                <Stack direction="column" spacing={1}>
                    {this.props.SSIDs.map(item =>
                        <Button variant={"outlined"}
                                startIcon={<WiFiIcon/>}
                                onClick={() => this.setState({
                                    credentials: {
                                        ssid: item,
                                        password: this.state.credentials.password,
                                    }
                                })}
                        >
                            {item}
                        </Button>
                    )}
                </Stack>
            </Box>
        )
    }

    renderCredentialsForm(): React.JSX.Element {
        return (
            <Box component="form" onSubmit={() => {
                this.setState({connTry: this.state.connTry + 1});
                this.props.onConnectRequest(this.state.credentials);
            }}>
                <Stack direction={"column"} spacing={1}>
                    {this.props.connStatus == ConnStatus.DISCONNECTED && this.state.connTry > 0 &&
                        <Typography variant={"subtitle1"} color={"error"}>
                            Connection failed, check the password
                        </Typography>
                    }
                    <TextField required autoFocus
                               label="Password"
                               variant="outlined"
                               type={"password"}
                               value={this.state.credentials.password}
                               disabled={this.props.connStatus == ConnStatus.CONNECTING}
                               onChange={e => this.setState({
                                   credentials: {
                                       ...this.state.credentials,
                                       password: (e.target as HTMLInputElement).value
                                   }
                               })}
                    />
                    <Button variant={"contained"}
                            startIcon={this.props.connStatus == ConnStatus.CONNECTING ?
                                <CircularProgress size={20}/> : <WiFiIcon/>}
                            disabled={!this.state.credentials.password.length || this.props.connStatus == ConnStatus.CONNECTING}
                    >
                        {this.props.connStatus == ConnStatus.CONNECTING ? "Connecting" : "Connect"} to {this.state.credentials.ssid}
                    </Button>
                    <Button variant={"outlined"}
                            startIcon={<ArrowBack/>}
                            disabled={this.props.connStatus == ConnStatus.CONNECTING}
                            onClick={() => {
                                this.setState({
                                    credentials: {ssid: "", password: ""},
                                    connTry: 0,
                                });
                            }}>
                        Other network
                    </Button>
                </Stack>
            </Box>
        )
    }

    renderConnected(): React.JSX.Element {
        return (
            <Stack direction={"column"} spacing={1}>
                {this.props.connectedSSID &&
                    <Typography variant={"subtitle1"} sx={{mb: 2}}>
                        Connected to <b>{this.props.connectedSSID}</b>
                    </Typography>
                }
                <Button variant={"contained"}
                        onClick={() => {
                            this.setState({
                                credentials: {ssid: "", password: ""},
                                connTry: 0,
                            });
                            this.props.onDisconnectRequest();
                        }}
                        disabled={this.props.connectedSSID == ""}
                >
                    Disconnect
                </Button>
            </Stack>
        )
    }

    render(): React.JSX.Element {
        if (!this.props.visible) {
            return <React.Fragment/>;
        }

        return (
            <React.Fragment>
                {
                    this.props.connStatus == ConnStatus.DISCONNECTED &&
                    this.state.credentials.ssid == "" &&
                    this.renderSSIDChooser()
                }
                {
                    this.props.connStatus != ConnStatus.CONNECTED &&
                    this.state.credentials.ssid != "" &&
                    this.renderCredentialsForm()
                }
                {
                    this.props.connStatus == ConnStatus.CONNECTED &&
                    this.renderConnected()
                }
            </React.Fragment>
        )
    }
}
