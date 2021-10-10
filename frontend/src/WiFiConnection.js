import {useState} from "react";

import Box from "@material-ui/core/Box";
import FormControl from "@material-ui/core/FormControl";
import InputLabel from "@material-ui/core/InputLabel";
import Select from "@material-ui/core/Select";
import MenuItem from "@material-ui/core/MenuItem";
import TextField from "@material-ui/core/TextField";
import CircularProgress from "@material-ui/core/CircularProgress";
import Button from "@material-ui/core/Button";
import Typography from "@material-ui/core/Typography";

import messages from "./messages";


export default function WiFiConnection(props) {
    const [ssid, setSSID] = useState("");
    const [password, setPassword] = useState("");

    return <Box hidden={!props.apConnected}>
        <h2 align={"center"}>{messages.t("titleWifiConnection")}</h2>

        <Box className={"wifiConnectionStatus"}>
            <p align={"center"}
               hidden={!props.wifiConnected}
               dangerouslySetInnerHTML={{__html: messages.t("wifiConnectedHelp", props.wifiConnInfo.ssid)}}/>

            <p hidden={props.wifiConnected} dangerouslySetInnerHTML={{__html: messages.t("wifiNotConnectedHelp")}}/>

            <Box className={"center"} hidden={!props.wifiConnected}>
                <Button variant="contained" onClick={props.wifiDisconnect} disabled={props.wifiConnInProgress}>
                    {messages.t('disconnect')}
                </Button>
            </Box>
        </Box>

        <Typography align={"center"} hidden={props.wifiConnected || props.wifiScanResult.length !== 0}>
            <CircularProgress className={'connProgress'}/>
        </Typography>

        <Box className={"wifiConnectionSettings"} hidden={props.wifiConnected || props.wifiScanResult.length === 0}>
            <Box>
                <FormControl fullWidth={true}>
                    <InputLabel id="ssid-select-label">{messages.t("ssid")}</InputLabel>
                    <Select
                        labelId="ssid-select-label"
                        id="ssid"
                        value={ssid}
                        disabled={props.wifiConnInProgress || !props.wifiScanResult.length}
                        onChange={e => setSSID(e.target.value)}
                    >
                        {props.wifiScanResult.map(item => {
                            return <MenuItem key={item[0]} value={item[0]}>{item[0]}</MenuItem>;
                        })}
                    </Select>
                </FormControl>
            </Box>

            <Box>
                <TextField type={"password"}
                           id={"password"}
                           label={messages.t("password")}
                           fullWidth={true}
                           disabled={props.wifiConnInProgress || !props.wifiScanResult.length}
                           onChange={e => setPassword(e.target.value)}
                />
            </Box>

            <Box className={"center"} hidden={!props.wifiConnInProgress}>
                <CircularProgress className={'wifiConnProgress'}/>
            </Box>

            <Box className={"center"} hidden={props.wifiConnInProgress}>
                <Button type={"submit"}
                        variant={"contained"}
                        disabled={!ssid}
                        onClick={() => props.wifiConnect(ssid, password)}
                >
                    {messages.t('connect')}
                </Button>
            </Box>
        </Box>
    </Box>
}
