package com.gland.medmanager

object NotificationMapper {

    fun fromFirebaseMessage(message: String): NotificationType {
        return when {
            message.contains("missed", ignoreCase = true) ->
                NotificationType.MISSED

            message.contains("late", ignoreCase = true) ->
                NotificationType.LATE

            message.contains("disconnected", ignoreCase = true) ->
                NotificationType.DISCONNECTED

            message.contains("refill", ignoreCase = true) ->
                NotificationType.REFILL

            else ->
                NotificationType.DEFAULT
        }
    }
}
