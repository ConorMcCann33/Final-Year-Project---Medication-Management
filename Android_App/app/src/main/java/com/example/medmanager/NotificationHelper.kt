package com.gland.medmanager

import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.core.app.NotificationCompat

object NotificationHelper {

    private const val CHANNEL_ID = "medmanager_notifications"

    fun showNotification(
        context: Context,
        type: NotificationType,
        fallbackMessage: String
    ) {
        val manager =
            context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "MedManager Alerts",
                NotificationManager.IMPORTANCE_HIGH
            )
            manager.createNotificationChannel(channel)
        }

        val contentText =
            if (type == NotificationType.DEFAULT) fallbackMessage else type.uiTitle

        val notification = NotificationCompat.Builder(context, CHANNEL_ID)
            .setSmallIcon(type.iconRes)
            .setContentTitle(type.pushTitle)
            .setContentText(contentText)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setAutoCancel(true)
            .build()

        manager.notify(System.currentTimeMillis().toInt(), notification)
    }
}
