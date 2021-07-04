const messages = {
    data: {
        "en": {
            "titleAlarm": "Alarm",
            "titleBrightness": "Brightness",
            "titleWifiConnection": "Wi-Fi Connection",

            "brightness": "Brightness",
            "maxBrightness": "Maximum brightness",

            "apConnHelp": "Cronus is not found. Connect your computer to 'Cronus' WiFi network " +
                "using password you've got with your device.",

            "wifiNotConnectedHelp": "Currently your Cronus is not connected to a WiFi network. " +
                "In order to receive correct time and weather updates you should connect Cronus to a WiFi network. " +
                "Please choose the network, enter the password and click 'Connect'.",
            "wifiConnectedHelp": "Currently your Cronus is connected to the Wifi network '{1}'.",
            "ssid": "Network",
            "rssi": "Signal strength",
            "password": "Password",
            "connect": "Connect",
            "disconnect": "Disconnect",
        },
        "ru": {

        },
        "uk": {

        },
    },

    t: (id, ...args) => {
        const lang = "en";

        let msg = messages.data[lang][id];
        if (msg === undefined) {
            return "[" + id + "]";
        }

        args.forEach((arg, i) => {
            msg = msg.replaceAll(`{${i + 1}}`, arg);
        });

        return msg;
    }
};

export default messages;
