package com.gland.medmanager

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.firebase.database.*

class LogFragment : Fragment() {

    private val logEntries = mutableListOf<LogEntry>()
    private lateinit var adapter: LogAdapter
    private lateinit var logRef: DatabaseReference
    private lateinit var logListener: ValueEventListener

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        return inflater.inflate(R.layout.fragment_log, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val recyclerView = view.findViewById<RecyclerView>(R.id.logRecyclerView)
        recyclerView.layoutManager = LinearLayoutManager(requireContext())
        recyclerView.setHasFixedSize(true)

        adapter = LogAdapter(logEntries)
        recyclerView.adapter = adapter

        logRef = FirebaseDatabase.getInstance()
            .getReference("devices")
            .child("device_001")
            .child("log")

        attachLogListener()
    }

    private fun attachLogListener() {
        logListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                logEntries.clear()

                for (child in snapshot.children) {
                    val entry = child.getValue(LogEntry::class.java)
                    if (entry != null && entry.alarmtime.isNotEmpty()) {
                        logEntries.add(entry)
                    }
                }

                adapter.notifyDataSetChanged()

            }

            override fun onCancelled(error: DatabaseError) {}
        }

        logRef.addValueEventListener(logListener)
    }

    override fun onDestroyView() {
        super.onDestroyView()
        logRef.removeEventListener(logListener)
    }
}
