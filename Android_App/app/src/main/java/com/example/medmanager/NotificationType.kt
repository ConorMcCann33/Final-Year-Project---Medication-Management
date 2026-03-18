package com.gland.medmanager

import androidx.annotation.ColorRes
import androidx.annotation.DrawableRes

enum class NotificationType(
    val uiTitle: String,
    @DrawableRes val iconRes: Int,
    @ColorRes val colorRes: Int,
    val pushTitle: String
) {
    MISSED(
        uiTitle = "Medication was Missed",
        iconRes = R.drawable.ic_alert,
        colorRes = R.color.red,
        pushTitle = "Medication Alert"
    ),

    LATE(
        uiTitle = "Medication was taken very late",
        iconRes = R.drawable.ic_alert,
        colorRes = R.color.orange,
        pushTitle = "Medication Alert"
    ),

    DISCONNECTED(
        uiTitle = "Device has Disconnected",
        iconRes = R.drawable.ic_device,
        colorRes = R.color.red,
        pushTitle = "Device Alert"
    ),

    REFILL(
        uiTitle = "Device needs to be Refilled",
        iconRes = R.drawable.ic_pill,
        colorRes = R.color.blue,
        pushTitle = "Refill Needed"
    ),

    DEFAULT(
        uiTitle = "",
        iconRes = R.drawable.ic_notifications,
        colorRes = R.color.gray,
        pushTitle = "MedManager Alert"
    )
}
