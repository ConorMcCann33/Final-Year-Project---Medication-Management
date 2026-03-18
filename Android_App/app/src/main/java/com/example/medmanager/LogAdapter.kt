package com.gland.medmanager

import android.graphics.Color
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class LogAdapter(private val logList: List<LogEntry>) :
    RecyclerView.Adapter<LogAdapter.LogViewHolder>() {

    class LogViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val logIndex: TextView = itemView.findViewById(R.id.tvLogIndex)
        val alarmTime: TextView = itemView.findViewById(R.id.tvAlarmTime)
        val timeTaken: TextView = itemView.findViewById(R.id.tvTimeTaken)
        val statusRow: LinearLayout = itemView.findViewById(R.id.statusRow)
        val statusIcon: ImageView = itemView.findViewById(R.id.statusIcon)
        val statusText: TextView = itemView.findViewById(R.id.statusText)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): LogViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.log_item, parent, false)
        return LogViewHolder(view)
    }

    override fun onBindViewHolder(holder: LogViewHolder, position: Int) {
        val entry = logList[position]

        holder.logIndex.text = "Log #${position + 1}"
        holder.alarmTime.text = entry.alarmtime
        holder.timeTaken.text = entry.timetaken

        when {
            entry.delayminutes < 15 -> {
                holder.statusText.text = "Medication taken on time"
                holder.statusText.setTextColor(Color.parseColor("#2E7D32"))
                holder.statusIcon.setImageResource(R.drawable.ic_alert)
            }
            entry.delayminutes in 15..30 -> {
                holder.statusText.text = "Medication taken late"
                holder.statusText.setTextColor(Color.parseColor("#F57C00"))
                holder.statusIcon.setImageResource(R.drawable.ic_alert)
            }
            else -> {
                holder.statusText.text = "Medication taken very late"
                holder.statusText.setTextColor(Color.parseColor("#C62828"))
                holder.statusIcon.setImageResource(R.drawable.ic_alert)
            }
        }
    }

    override fun getItemCount(): Int = logList.size
}
