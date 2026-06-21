//////////////////////////////////////////////////////
// RC DATA
//////////////////////////////////////////////////////

let rc = {
    thr: 0,
    yaw: 0,
    pit: 0,
    rol: 0,
    arm: false
};

//////////////////////////////////////////////////////
// WebSocket
//////////////////////////////////////////////////////

let ws = null;
let lastPing = 0;

function connectWS()
{
    let protocol =
        location.protocol === "https:"
        ? "wss"
        : "ws";

    ws = new WebSocket(
        `${protocol}://${location.hostname}:81`
    );

    ws.onopen = () =>
    {
    	console.log("WS Connected");
        connectionStatus.textContent =
            "CONNECTED";

        connectionStatus.className =
            "status connected";
    };

    ws.onclose = () =>
    {
    	console.log("WS Closed");
        connectionStatus.textContent =
            "DISCONNECTED";

        connectionStatus.className =
            "status disconnected";

        setTimeout(connectWS, 1000);
    };

	ws.onerror = (e) => {
		console.log("WS Error", e);
	};
	
    ws.onmessage = (event) =>
    {
    	//console.log("WS RX", event.data);
    	
        const data =
            JSON.parse(event.data);

        updateTelemetry(data);

        latencyValue.textContent =
            Date.now() - lastPing;
    };
}

//////////////////////////////////////////////////////
// SEND
//////////////////////////////////////////////////////

function sendControl()
{
    if(!ws) return;

    if(ws.readyState !== WebSocket.OPEN)
        return;

    lastPing = Date.now();

    ws.send(JSON.stringify(rc));
}

setInterval(sendControl, 20);

//////////////////////////////////////////////////////
// ARM/DISARM
//////////////////////////////////////////////////////
armToggleButton.onclick = () =>
{
    rc.arm = !rc.arm;

    if(rc.arm)
    {
        armToggleButton.textContent = "DISARM";

        armStatus.textContent = "ARMED";
        armStatus.className = "status armed";
    }
    else
    {
        armToggleButton.textContent = "ARM";

        armStatus.textContent = "DISARMED";
        armStatus.className = "status disarmed";
    }
};


//////////////////////////////////////////////////////
// TELEMETRY
//////////////////////////////////////////////////////

function setMotorBar(id, value)
{
    const percent =
        (value / 255) * 100;

    document
        .getElementById(id)
        .style.width =
        percent + "%";
}

function updateTelemetry(data)
{
    m1Value.textContent = data.m1;
    m2Value.textContent = data.m2;
    m3Value.textContent = data.m3;
    m4Value.textContent = data.m4;

    setMotorBar("m1Bar", data.m1);
    setMotorBar("m2Bar", data.m2);
    setMotorBar("m3Bar", data.m3);
    setMotorBar("m4Bar", data.m4);

    rssiValue.textContent =
        data.rssi;
}

//////////////////////////////////////////////////////
// DEBUG
//////////////////////////////////////////////////////

function updateDebug()
{
    thrDebug.textContent = rc.thr;
    yawDebug.textContent = rc.yaw;
    pitDebug.textContent = rc.pit;
    rolDebug.textContent = rc.rol;
}

setInterval(updateDebug, 50);

//////////////////////////////////////////////////////
// JOYSTICK
//////////////////////////////////////////////////////

class VirtualStick
{
    constructor(
        canvasId,
        options
    )
    {
        this.canvas = document.getElementById(canvasId);
        this.ctx = this.canvas.getContext("2d");
		this.radius = this.canvas.width * 0.38;
		this.knobRadius = this.canvas.width * 0.13;
        this.centerX =  this.canvas.width / 2;
        this.centerY = this.canvas.height / 2;

		this.x = 0;

		if(options.type==="left")
		{
		    this.y = 1; // 左スティック初期位置を下端
		}
		else
		{
		    this.y = 0;
		}
        
        this.touchId = null;

        this.dragging = false;

        this.options = options;

        this.installEvents();

        this.draw();
    }

    installEvents()
    {
        this.canvas.addEventListener(
            "touchstart",
            (e) =>
            {
                e.preventDefault();
                const touch = e.changedTouches[0];

				this.touchId = touch.identifier;
				this.dragging = true;
            }
        );

        this.canvas.addEventListener(
            "touchmove",
            (e) =>
            {
                e.preventDefault();

                if(!this.dragging)
                    return;

                const rect =
				    this.canvas.getBoundingClientRect();

				const centerX =
				    rect.width / 2;

				const centerY =
				    rect.height / 2;

				const radius =
				    Math.min(
				        rect.width,
				        rect.height
				    ) / 2 - 10;

				let t = null;

				for(const touch of e.touches)
				{
				    if(touch.identifier === this.touchId)
				    {
				        t = touch;
				        break;
				    }
				}

				if(!t)
				    return;

				let dx =
				    t.clientX -
				    rect.left -
				    centerX;

				let dy =
				    t.clientY -
				    rect.top -
				    centerY;

				const len =
				    Math.sqrt(
				        dx * dx +
				        dy * dy
				    );

				if(len > radius)
				{
				    dx *= radius / len;
				    dy *= radius / len;
				}

				this.x = dx / radius;
				this.y = dy / radius;

                this.updateRC();

                this.draw();
            }
        );

        this.canvas.addEventListener(
            "touchend",
            (e) =>
            {
                e.preventDefault();

				for(const touch of e.changedTouches)
				{
				    if(touch.identifier === this.touchId)
				    {
				        this.dragging = false;
				        this.touchId = null;

				        this.release();
				        this.draw();

				        break;
				    }
				}

                this.draw();
            }
        );
    }

	release()
	{
	    if(this.options.type === "left")
	    {
	        // YAWだけセンターへ戻す
	        this.x = 0;

	        // スロットル位置維持
	        rc.yaw = 0;
	    }
	    else
	    {
	        // 右スティックは両方センター復帰
	        this.x = 0;
	        this.y = 0;

	        rc.rol = 0;
	        rc.pit = 0;
	    }

	    this.updateRC();
	}

    updateRC()
    {
        if(this.options.type === "left")
        {
            rc.yaw =
                Math.round(
                    this.x * 100
                );

            rc.thr =
                Math.max(
                    0,
                    Math.min(
                        255,
                        Math.round(
                            (-this.y + 1)
                            * 127.5
                        )
                    )
                );
        }
        else
        {
            rc.rol =
                Math.round(
                    this.x * 100
                );

            rc.pit =
                Math.round(
                    -this.y * 100
                );
        }
    }

    draw()
    {
        const ctx = this.ctx;

        ctx.clearRect(
            0,
            0,
            this.canvas.width,
            this.canvas.height
        );

        ctx.strokeStyle =
            "#00ffff";

        ctx.lineWidth = 2;

        ctx.beginPath();

        ctx.arc(
            this.centerX,
            this.centerY,
            this.radius,
            0,
            Math.PI*2
        );

        ctx.stroke();

        const knobX =
            this.centerX +
            this.x * this.radius;

        const knobY =
            this.centerY +
            this.y * this.radius;

        ctx.fillStyle =
            "#00ffff";

        ctx.beginPath();

        ctx.arc(
            knobX,
            knobY,
            this.knobRadius,
            0,
            Math.PI*2
        );

        ctx.fill();
    }
}

//////////////////////////////////////////////////////
// STICKS
//////////////////////////////////////////////////////

const leftStick =
    new VirtualStick(
        "leftStick",
        {type:"left"}
    );

const rightStick =
    new VirtualStick(
        "rightStick",
        {type:"right"}
    );

//////////////////////////////////////////////////////
// START
//////////////////////////////////////////////////////

connectWS();