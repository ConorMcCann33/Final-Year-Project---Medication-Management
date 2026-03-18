package com.gland.medmanager

import android.os.Bundle
import android.view.*
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.tabs.TabLayout
import com.google.firebase.database.*

class NotificationFragment : Fragment() {

    private val allNotifications = mutableListOf<NotificationEntry>()
    private val filteredNotifications = mutableListOf<NotificationEntry>()

    private lateinit var adapter: NotificationAdapter
    private lateinit var logNotRef: DatabaseReference

    private var selectedDays = 1

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return inflater.inflate(R.layout.fragment_notifications, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {

        val recycler = view.findViewById<RecyclerView>(R.id.notificationRecycler)
        recycler.layoutManager = LinearLayoutManager(requireContext())
        adapter = NotificationAdapter(filteredNotifications)
        recycler.adapter = adapter

        val tabs = view.findViewById<TabLayout>(R.id.filterTabs)

        tabs.addTab(tabs.newTab().setText("24 Hours"), true)
        tabs.addTab(tabs.newTab().setText("7 Days"))
        tabs.addTab(tabs.newTab().setText("28 Days"))
        tabs.addTab(tabs.newTab().setText("3 Months"))

        tabs.addOnTabSelectedListener(object : TabLayout.OnTabSelectedListener {
            override fun onTabSelected(tab: TabLayout.Tab) {
                selectedDays = when (tab.position) {
                    0 -> 1
                    1 -> 7
                    2 -> 28
                    else -> 90
                }
                applyFilter()
            }

            override fun onTabUnselected(tab: TabLayout.Tab) {}
            override fun onTabReselected(tab: TabLayout.Tab) {}
        })

        logNotRef = FirebaseDatabase.getInstance()
            .getReference("devices")
            .child("device_001")
            .child("log_not")

        attachRealtimeListener()
    }

    private fun attachRealtimeListener() {
        logNotRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {

                allNotifications.clear()

                val now = System.currentTimeMillis()
                val cutoff90 = now - 90L * 24 * 60 * 60 * 1000

                for (child in snapshot.children) {
                    val entry = child.getValue(NotificationEntry::class.java) ?: continue

                    if (entry.timestamp < cutoff90) {
                        child.ref.removeValue()
                    } else {
                        allNotifications.add(entry)
                    }
                }

                allNotifications.sortByDescending { it.timestamp }
                applyFilter()
            }

            override fun onCancelled(error: DatabaseError) {}
        })
    }

    private fun applyFilter() {
        val cutoff = System.currentTimeMillis() - selectedDays * 24L * 60 * 60 * 1000
        filteredNotifications.clear()
        filteredNotifications.addAll(allNotifications.filter { it.timestamp >= cutoff })
        adapter.notifyDataSetChanged()
    }
}
