package com.gland.medmanager

import android.os.Bundle
import android.os.Handler
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.firebase.database.*

class MainActivity : AppCompatActivity() {

    private lateinit var notifyRef: DatabaseReference
    private lateinit var heartbeatRef: DatabaseReference

    private var lastHeartbeatTime = 0L
    private var heartbeatReceived = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Default fragment
        if (savedInstanceState == null) {
            openFragment(HomeFragment())
        }

        if (android.os.Build.VERSION.SDK_INT >= 33) {
            requestPermissions(
                arrayOf(android.Manifest.permission.POST_NOTIFICATIONS),
                1001
            )
        }

        val deviceRef = FirebaseDatabase.getInstance()
            .getReference("devices")
            .child("device_001")

        notifyRef = deviceRef.child("status").child("notify")
        heartbeatRef = deviceRef.child("status").child("heartbeat")

        attachNotifyListener()
        attachHeartbeatListener()
        startHeartbeatMonitor()

        // Bottom navigation
        val bottomNav = findViewById<BottomNavigationView>(R.id.bottom_navigation)
        bottomNav.setOnItemSelectedListener { item ->
            when (item.itemId) {
                R.id.nav_home -> {
                    openFragment(HomeFragment())
                    true
                }
                R.id.nav_log -> {
                    openFragment(LogFragment())
                    true
                }
                R.id.nav_notifications -> {
                    openFragment(NotificationFragment())
                    true
                }
                else -> false
            }
        }
    }

    private fun openFragment(fragment: androidx.fragment.app.Fragment) {
        supportFragmentManager.beginTransaction()
            .replace(R.id.fragment_container, fragment)
            .commit()
    }

    private fun attachNotifyListener() {
        notifyRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val rawMessage = snapshot.getValue(String::class.java) ?: return
                if (rawMessage.lowercase() == "none") return

                val type = NotificationMapper.fromFirebaseMessage(rawMessage)

                NotificationHelper.showNotification(
                    context = this@MainActivity,
                    type = type,
                    fallbackMessage = rawMessage
                )
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }


    private fun attachHeartbeatListener() {
        heartbeatRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                val heartbeatString = snapshot.getValue(String::class.java) ?: return
                val heartbeat = heartbeatString.toLongOrNull() ?: return

                heartbeatReceived = true
                lastHeartbeatTime = heartbeat

                println("Heartbeat received: $heartbeat")
                onDeviceOnline()
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }

    private fun startHeartbeatMonitor() {
        val handler = Handler(mainLooper)

        val runnable = object : Runnable {
            override fun run() {
                val now = System.currentTimeMillis()

                if (heartbeatReceived && now - lastHeartbeatTime > 60_000) {
                    onDeviceOffline()
                }

                handler.postDelayed(this, 15_000)
            }
        }

        handler.post(runnable)
    }

    private fun onDeviceOnline() {
        println("Device ONLINE")
        // Update UI here
    }

    private fun onDeviceOffline() {
        println("Device OFFLINE")
        // Update UI here
    }
}
