package com.gland.medmanager

import android.util.Log
import com.google.firebase.messaging.FirebaseMessagingService
import com.google.firebase.messaging.RemoteMessage

class MyFirebaseMessagingService : FirebaseMessagingService() {

    override fun onMessageReceived(message: RemoteMessage) {
        super.onMessageReceived(message)

        // Prefer data payload, fallback to notification body
        val rawMessage =
            message.data["message"]
                ?: message.notification?.body
                ?: return

        val type = NotificationMapper.fromFirebaseMessage(rawMessage)

        NotificationHelper.showNotification(
            context = this,
            type = type,
            fallbackMessage = rawMessage
        )

        Log.d("FCM", "Push received: $rawMessage")
    }

    override fun onNewToken(token: String) {
        super.onNewToken(token)
        Log.d("FCM", "FCM Token: $token")
    }
}
