const { onValueWritten } = require("firebase-functions/v2/database");
const admin = require("firebase-admin");

admin.initializeApp();

exports.notifyHandler = onValueWritten(
  {
    ref: "/devices/device_001/status/notify",
    region: "europe-west1"
  },
  async (event) => {
    const newValue = event.data.after.val();

    if (!newValue || newValue.toLowerCase() === "none") {
      return;
    }

    console.log("Notify received:", newValue);

    const message = {
      data: {
        message: newValue
      },
      topic: "device_001"
    };



    await admin.messaging().send(message);

    const logRef = admin.database()
      .ref("/devices/device_001/log_not");

    await logRef.push({
      message: newValue,
      timestamp: Date.now()
    });

    await admin.database()
      .ref("/devices/device_001/status/notify")
      .set("none");

    return;
  }
);
