package com.gland.medmanager

import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.google.firebase.database.*
import java.text.SimpleDateFormat
import java.util.*

class HomeFragment : Fragment() {

    private lateinit var ref: DatabaseReference

    // Device & Alert
    private lateinit var deviceStatusText: TextView
    private lateinit var lastSeenText: TextView
    private lateinit var alertStatusText: TextView
    private lateinit var deviceIcon: ImageView
    private lateinit var alertIcon: ImageView

    // Alarms
    private lateinit var alarm1: TextView
    private lateinit var alarm2: TextView
    private lateinit var alarm3: TextView
    private lateinit var alarm4: TextView

    // Latest Medication Log (reusing log_item views)
    private lateinit var latestAlarmTime: TextView
    private lateinit var latestTimeTaken: TextView
    private lateinit var latestStatusText: TextView
    private lateinit var latestStatusIcon: ImageView
    private lateinit var latestStatusRow: LinearLayout

    // Latest Notification
    private lateinit var latestNotificationTitle: TextView
    private lateinit var latestNotificationTime: TextView
    private lateinit var latestNotificationIcon: ImageView

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?
    ): View {
        val v = inflater.inflate(R.layout.fragment_home, container, false)

        // Device / Alert
        deviceStatusText = v.findViewById(R.id.deviceStatusText)
        lastSeenText = v.findViewById(R.id.lastSeenText)
        alertStatusText = v.findViewById(R.id.alertStatusText)
        deviceIcon = v.findViewById(R.id.deviceStatusIcon)
        alertIcon = v.findViewById(R.id.alertStatusIcon)

        // Alarms
        alarm1 = v.findViewById(R.id.alarm1Time)
        alarm2 = v.findViewById(R.id.alarm2Time)
        alarm3 = v.findViewById(R.id.alarm3Time)
        alarm4 = v.findViewById(R.id.alarm4Time)

        // Latest Medication Log (using included layout IDs)
        latestAlarmTime = v.findViewById(R.id.tvAlarmTime)
        latestTimeTaken = v.findViewById(R.id.tvTimeTaken)
        latestStatusText = v.findViewById(R.id.statusText)
        latestStatusIcon = v.findViewById(R.id.statusIcon)
        latestStatusRow = v.findViewById(R.id.statusRow)

        // Latest Notification
        latestNotificationTitle = v.findViewById(R.id.latestNotificationTitle)
        latestNotificationTime = v.findViewById(R.id.latestNotificationTime)
        latestNotificationIcon = v.findViewById(R.id.latestNotificationIcon)

        // Firebase reference
        ref = FirebaseDatabase.getInstance().getReference("devices/device_001")

        listenStatus()
        listenAlarms()
        listenLatestLog()
        listenLatestNotification()

        return v
    }

    private fun listenStatus() {
        ref.child("status").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(s: DataSnapshot) {
                val internet = s.child("internet").getValue(String::class.java) ?: "Offline"
                val heartbeat = s.child("heartbeat").getValue(String::class.java)?.toLongOrNull() ?: 0L
                val diff = System.currentTimeMillis() - heartbeat

                if (diff < 60_000) {
                    deviceStatusText.text = "Online"
                    lastSeenText.text = ""
                } else {
                    deviceStatusText.text = "Offline"
                    lastSeenText.text = formatAgo(diff)
                }

                alertStatusText.text = s.child("alert").getValue(String::class.java) ?: "Not Active"
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }

    private fun listenAlarms() {
        ref.child("alarms").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(s: DataSnapshot) {
                alarm1.text = "Alarm 1: ${s.child("alarm1").value ?: "--:--"}"
                alarm2.text = "Alarm 2: ${s.child("alarm2").value ?: "--:--"}"
                alarm3.text = "Alarm 3: ${s.child("alarm3").value ?: "--:--"}"
                alarm4.text = "Alarm 4: ${s.child("alarm4").value ?: "--:--"}"
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }

    private fun listenLatestLog() {
        ref.child("log").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {

                // Default placeholders if no log exists
                var alarm = "--:--"
                var taken = "--:--"
                var delay = 0

                if (snapshot.hasChildren()) {
                    val last = snapshot.children.first()
                    alarm = last.child("alarmtime").getValue(String::class.java) ?: "--:--"
                    taken = last.child("timetaken").getValue(String::class.java) ?: "--:--"
                    delay = (last.child("delayminutes").getValue(Long::class.java) ?: 0L).toInt()
                }

                // Set values
                latestAlarmTime.text = alarm
                latestTimeTaken.text = taken

                when {
                    delay < 15 -> {
                        latestStatusText.text = "Medication taken on time"
                        latestStatusText.setTextColor(Color.parseColor("#2E7D32"))
                        latestStatusIcon.setImageResource(R.drawable.ic_alert)
                    }
                    delay in 15..30 -> {
                        latestStatusText.text = "Medication taken late"
                        latestStatusText.setTextColor(Color.parseColor("#F57C00"))
                        latestStatusIcon.setImageResource(R.drawable.ic_alert)
                    }
                    else -> {
                        latestStatusText.text = "Medication taken very late"
                        latestStatusText.setTextColor(Color.parseColor("#C62828"))
                        latestStatusIcon.setImageResource(R.drawable.ic_alert)
                    }
                }
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }

    private fun listenLatestNotification() {
        ref.child("log_not").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {

                var latestMsg = ""
                var latestTime = 0L

                for (child in snapshot.children) {
                    val ts = child.child("timestamp").getValue(Long::class.java) ?: continue
                    if (ts > latestTime) {
                        latestTime = ts
                        latestMsg = child.child("message").getValue(String::class.java) ?: ""
                    }
                }

                if (latestTime > 0) {
                    val type = NotificationMapper.fromFirebaseMessage(latestMsg)

                    latestNotificationTitle.text = type.uiTitle.ifEmpty { latestMsg }

                    val diff = System.currentTimeMillis() - latestTime
                    val agoText = if (diff < 60_000) "Just now" else formatAgo(diff).replace("Last seen ", "")
                    latestNotificationTime.text = agoText

                    latestNotificationIcon.setImageResource(type.iconRes)
                    latestNotificationIcon.setColorFilter(requireContext().getColor(type.colorRes))
                } else {
                    latestNotificationTitle.text = "—"
                    latestNotificationTime.text = ""
                    latestNotificationIcon.setImageResource(R.drawable.ic_notifications)
                    latestNotificationIcon.setColorFilter(requireContext().getColor(R.color.gray))
                }
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }
    private fun formatAgo(diff: Long): String {
        val mins = diff / 60_000
        val hrs = diff / 3_600_000
        val days = diff / 86_400_000

        return when {
            mins < 60 -> "Last seen $mins min${if (mins != 1L) "s" else ""} ago"
            hrs < 24 -> "Last seen $hrs hour${if (hrs != 1L) "s" else ""} ago"
            else -> "Last seen $days day${if (days != 1L) "s" else ""} ago"
        }
    }
}