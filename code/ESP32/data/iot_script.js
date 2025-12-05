const char iot_script[] PROGMEM = R"rawliteral(
const NetworkSelection = ["NotConnected", "APMode", "WiFiMode", "EthernetMode", "ModemMode"];

const NetworkSelectionEnum = {
    NotConnected: 0,
    APMode: 1,
    WiFiMode: 2,
    EthernetMode: 3,
    ModemMode: 4
};

const ModbusMode = ["tcp", "rtu"];

const ModbusModeEnum = {
    tcp: 0,
    rtu: 1
};

const UART_Parity = ["none", "", "even", "odd"];
const UART_ParityEnum = {
    none: 0,
    even: 2,
    odd: 3
};

const UART_StopBits = ["", "1", "1.5", "2", "max"];
const UART_StopBitsEnum = {
    1: 1,
    2: 3,
};

function dhcpCheck(checkbox) {
    const fieldset = document.getElementById("dhcp");
    fieldset.disabled = checkbox.checked;
}

function mqttFieldset(checkbox) {
    const fieldset = document.getElementById("mqtt");
    fieldset.disabled = !checkbox.checked;
}

function modbusFieldset(checkbox) {
    const fieldset = document.getElementById("modbus");
    fieldset.disabled = !checkbox.checked;
}

function showMBFields() {
    document.querySelectorAll('#modbusSelector-container > div').forEach(div => div.classList.add('hidden'));
    var selectedFld = document.getElementById('modbusModeSelector');
    if (selectedFld) {
        document.getElementById(selectedFld.value + '-fields').classList.remove('hidden');
    }
}

function showFields() {
    // Hide all fields
    document.querySelectorAll('#networkSelector-container > div').forEach(div => div.classList.add('hidden'));
    // Get selected value
    var selectedValue = document.getElementById('networkSelector').value.trim();
    if (selectedValue === "APMode") {
        var modbusEl = document.getElementById('modbus');
        if (modbusEl) {
            modbusEl.classList.add('hidden');
        }
        var mqttEl = document.getElementById('mqtt');
        if (mqttEl) {
            mqttEl.classList.add('hidden');
        }
    }
    else {
        document.getElementById(selectedValue + '-fields').classList.remove('hidden');
        var mqttEl = document.getElementById('mqtt');
        if (mqttEl) {
            mqttEl.classList.remove('hidden');
        }
        var modbusEl = document.getElementById('modbus');
        if (modbusEl) {
            modbusEl.classList.remove('hidden');
        }
        showMBFields();
    }
}

window.onload = function () {
    loadSettings();
    %onload%
    const form = document.getElementById('settingsForm');
    if (form) {
        form.addEventListener('submit', async (e) => {
            e.preventDefault();
            const status = document.getElementById('status');
            status.textContent = '';
            %app_script_js%
            try {
                const iot = getFormValues("#iot_fields input, #iot_fields select");
                const res = await fetch('/iot_fields', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(iot)
                });
                console.log(await res.text());
                if (!res.ok) throw new Error('Save failed');
                status.innerHTML = '<span class="ok">IOT Settings saved.</span>';
            } catch (e) {
                status.innerHTML = '<span class="err">' + e.message + '</span>';
            }
        });

    }
}
    
function validateInputs() {
    %validateInputs%
    return true;
}

async function loadSettings() {
    const status = document.getElementById('status');
    try {
        const res = await fetch('/appsettings', { method: 'GET' });
        if (!res.ok) throw new Error('Failed to load');
        const cfg = await res.json();
        setAppValues(cfg);
    } catch (e) {
        status.innerHTML = '<span class="err">Load failed.</span>';
    }
    try {
        const res = await fetch('/iot_fields', { method: 'GET' });
        if (!res.ok) throw new Error('Failed to load');
        const cfg = await res.json();
        setIotValues(cfg);
        showFields();
        var mqttEl = document.getElementById('mqttCheckbox');
        if (mqttEl) {
            mqttFieldset(mqttEl);
        }
        var modbusEl = document.getElementById('modbusCheckbox');
        if (modbusEl) {
            modbusFieldset(modbusEl);
        }
        var dhcpEl = document.getElementById('useDHCP');
        if (dhcpEl) {
            dhcpCheck(dhcpEl);
        }
    } catch (e) {
        status.innerHTML = '<span class="err">Load failed.</span>';
    }
}

function setIotValues(cfg) {
    const form = document.getElementById('settingsForm');
    for (const [k, v] of Object.entries(cfg)) {
        const el = form.elements.namedItem(k);
        if (!el) continue;
        if (el.type === 'checkbox') {
            el.checked = !!v;
        } else if (el.tagName === 'SELECT') {
            // Special handling for select elements with enum values
            if (k === "Network") {
                el.value = NetworkSelection[v];
            } else if (k === "modbusMode") {
                el.value = ModbusMode[v];
            } else if (k === "svrRTUParity") {
                el.value = UART_Parity[v];
            } else if (k === "svrRTUStopBits") {
                el.value = UART_StopBits[v];
            } else {
                el.value = v;
            }
        } else {
            el.value = v;
        }
    }
}

function setAppValues(cfg, prefix = "") {
  const form = document.getElementById("settingsForm");

  for (const [k, v] of Object.entries(cfg)) {
    const keyPath = prefix ? `${prefix}.${k}` : k;

    if (Array.isArray(v)) {
      // Handle arrays: expect form fields named like conversions[0].minV
      v.forEach((item, i) => {
        if (typeof item === "object") {
          // recurse into object elements
          setAppValues(item, `${keyPath}[${i}]`);
        } else {
          const el = form.elements.namedItem(`${keyPath}[${i}]`);
          if (el) el.value = item;
        }
      });
    } else if (typeof v === "object" && v !== null) {
      // Handle nested objects
      setAppValues(v, keyPath);
    } else {
      // Handle scalars
      const el = form.elements.namedItem(keyPath);
      if (!el) continue;
      if (el.type === "checkbox") {
        el.checked = !!v;
      } else {
        el.value = v;
      }
    }
  }
}

function getFormValues(section) {
    const networkFields = document.querySelectorAll(section)
    const data = {};
    for (const el of networkFields) {
        if (!el.name) continue;
        if (el.type === 'checkbox') {
            data[el.name] = el.checked;
        } else if (el.type === 'number') {
            data[el.name] = el.value === '' ? null : Number(el.value);
        } else if (el.tagName === 'SELECT') {
            // Special handling for select elements with enum values
            if (el.name === "Network") {
                data[el.name] = NetworkSelectionEnum[el.value];
            } else if (el.name === "modbusMode") {
                data[el.name] = ModbusModeEnum[el.value];
            } else if (el.name === "svrRTUParity") {
                data[el.name] = UART_ParityEnum[el.value];
            } else if (el.name === "svrRTUStopBits") {
                data[el.name] = UART_StopBitsEnum[el.value];
            } 
            else {
                 data[el.name] = el.value;
            }
        } else {
            data[el.name] = el.value;
        }
    }
    return data;
}

%appScript%

)rawliteral";
