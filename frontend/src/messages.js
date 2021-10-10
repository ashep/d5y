const messages = {
    data: {
        "en": {
            "titleWifiConnection": "Network Connection",
            "apConnHelp": "Make sure you device is turned on, then connect your computer or smartphone " +
                "to its Wi-Fi hotspot using name and password you've got with your device.",
            "wifiNotConnectedHelp": "Your device is not connected to a network. " +
                "Please choose the network, enter the password and click 'Connect'.",
            "wifiConnectedHelp": "The device is connected to the <b>'{1}'</b> network.",
            "ssid": "Network",
            "password": "Password",
            "connect": "Connect",
            "disconnect": "Disconnect",
        },
        "ru": {
            "titleWifiConnection": "Подключение к сети",
            "apConnHelp": "Убедитесь, что устройство включено, после чего подключите свой компьютер или смартфон " +
                "к точке доступа, используя имя и пароль, которые вы получили с устройством.",
            "wifiNotConnectedHelp": "Устройство не подключено к сети. " +
                "Выберите сеть, введите пароль и нажмите кнопку 'Подключиться'",
            "wifiConnectedHelp": "Устройство подключено к сети <b>'{1}'</b>.",
            "ssid": "Сеть",
            "password": "Пароль",
            "connect": "Подключиться",
            "disconnect": "Отключиться",
        },
        "uk": {
            "titleWifiConnection": "Підключення до мережі",
            "apConnHelp": "Переконайтеся, що пристрій увімкнено, після чого підключіть свій комп'ютер або смартфон" +
                "до точки доступу, використовуючи ім'я та пароль, які ви отримали з пристроєм.",
            "wifiNotConnectedHelp": "Пристрій не підключений до мережі. " +
                "Виберіть мережу, введіть пароль і натисніть кнопку 'Підключитися'",
            "wifiConnectedHelp": "Пристрій підключено до мережі <b>'{1}'</b>.",
            "ssid": "Мережа",
            "password": "Пароль",
            "connect": "Підключитися",
            "disconnect": "Відключитися",
        },
    },

    t: (id, ...args) => {
        let lang = window.navigator.language;

        lang = lang.substring(0, 2);

        if (!["en", "ru", "uk"].includes(lang)) {
            lang = "en";
        }

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
