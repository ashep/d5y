import React, {useEffect, useState} from 'react';

const deviceBaseURL = "http://192.168.4.1";

function deviceRequest(method, endpoint, reqData, onSuccess, onError) {
    const opts = {
        method: method,
        mode: "cors",
    };

    if (method === "POST") {
        opts["headers"] = {'Content-Type': 'application/json'};

        if (reqData) {
            opts["body"] = JSON.stringify(reqData);
        }
    }

    fetch(deviceBaseURL + endpoint, opts).then(resp => {
        if (resp.status !== 200) {
            console.warn("bad response status: " + resp.status);
        } else {
            resp.json().then(data => {
                onSuccess(data)
            }).catch(err => {
                if (onError !== undefined) {
                    onError(err);
                }
            });
        }
    }).catch(err => {
        if (onError !== undefined) {
            onError(err);
        }
    })
}

function updateConnInfo(setAPConnected, setWifiConnInfo, setWifiConnected, setWifiConnInProgress) {
    deviceRequest("GET", "/wifi", null,
        data => {
            setAPConnected(true);
            setWifiConnInProgress(false);
            if (data.result) {
                setWifiConnected(true);
                setWifiConnInfo(data.result);
            } else {
                setWifiConnected(false);
                setWifiConnInfo({});
            }
            setTimeout(() => updateConnInfo(setAPConnected, setWifiConnInfo, setWifiConnected, setWifiConnInProgress), 5000);
        },
        err => {
            setAPConnected(false);
            setWifiConnected(false);
            setWifiConnInfo({});
            setTimeout(() => updateConnInfo(setAPConnected, setWifiConnInfo, setWifiConnected, setWifiConnInProgress), 1000);
        }
    );
}

function wifiConnect(ssid, password, setWifiConnInProgress) {
    setWifiConnInProgress(true);

    deviceRequest("POST", "/wifi", {ssid: ssid, password: password},
        data => {
            deviceRequest("POST", "/wifi/connect", {ssid: ssid, password: password},
                data => {
                },
                err => {
                    setWifiConnInProgress(false);
                    console.error(err);
                }
            );
        },
        err => {
            setWifiConnInProgress(false);
            console.error(err);
        }
    );
}

function wifiDisconnect(setWifiConnInProgress, setWifiConnected) {
    setWifiConnInProgress(true);

    deviceRequest("POST", "/wifi/disconnect", null,
        data => {
            if (data.result) {
                setWifiConnected(false);
            }
            setWifiConnInProgress(false);
        },
        err => {
            setWifiConnInProgress(false);
            console.error(err);
        });
}

function wifiScan(apConnected, wifiConnected, setWifiScanResult) {
    if (apConnected && !wifiConnected) {
        deviceRequest("POST", "/wifi/scan", null,
            data => {
                setWifiScanResult(data.result);
            },
            err => {
                console.error(err);
            }
        );
    }
}

export default function Connection(props) {
    const [apConnected, setAPConnected] = useState(false);
    const [wifiConnInProgress, setWifiConnInProgress] = useState(false);
    const [wifiConnected, setWifiConnected] = useState(false);
    const [wifiScanResult, setWifiScanResult] = useState([]);
    const [wifiConnInfo, setWifiConnInfo] = useState({});

    useEffect(() => {
        updateConnInfo(setAPConnected, setWifiConnInfo, setWifiConnected, setWifiConnInProgress);
    }, []);

    useEffect(() => {
        wifiScan(apConnected, wifiConnected, setWifiScanResult);
    }, [apConnected, wifiConnected]);

    return <>
        {React.Children.map(props.children, child => {
            if (React.isValidElement(child)) {
                return React.cloneElement(child, {
                    apConnected: apConnected,
                    setAPConnected: setAPConnected,
                    wifiConnInProgress: wifiConnInProgress,
                    setWifiConnInProgress: setWifiConnInProgress,
                    wifiConnected: wifiConnected,
                    setWifiConnected: setWifiConnected,
                    wifiScanResult: wifiScanResult,
                    setWifiScanResult: setWifiScanResult,
                    wifiConnInfo: wifiConnInfo,
                    setWifiConnInfo: setWifiConnInfo,
                    wifiConnect: (ssid, password) => wifiConnect(ssid, password, setWifiConnInProgress),
                    wifiDisconnect: () => wifiDisconnect(setWifiConnInProgress, setWifiConnected),
                });
            }
            return child;
        })}
    </>
}
