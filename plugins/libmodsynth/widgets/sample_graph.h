/* 
 * File:   sample_graph.h
 * Author: Jeff Hubbard
 * 
 * Display a graph of an audio file in a QPixMap.
 *
 * Created on April 3, 2012, 8:40 PM
 */

#ifndef SAMPLE_GRAPH_H
#define	SAMPLE_GRAPH_H

#include <QPixmap>
#include <QPainter>
#include <QList>
#include <sndfile.h>

class LMS_sample_graph
{
public:
    LMS_sample_graph(int a_count, int a_graph_height, int a_graph_width)
    {
        lms_graph_height = a_graph_height;
        lms_graph_width = a_graph_width;
        lms_graph_count = a_count;
        
        for(int f_i = 0; f_i < a_count; f_i++)
        {
            QPixmap * f_pixmap;
            f_pixmap->fill();
            lms_graphs.append(f_pixmap);
            lms_durations.append(0);
        }
    }
        
    QList <QPixmap*> lms_graphs;
    QList <int> lms_durations;
    int lms_graph_count;
    int lms_graph_height;
    int lms_graph_width;
       

    void generatePreview(QString path, int a_index)
    {
        if(a_index > lms_graph_count)
        {
            std::cout << "a_index is greater than lms_graph_count, NOT generating a sample graph\n";
        }
        
        SF_INFO info;
        SNDFILE *file;
        
        lms_graphs[a_index].fill();

        printf("set sample index\n");

        info.format = 0;
        file = sf_open(path.toLocal8Bit(), SFM_READ, &info);
        printf("Opened SNDFILE\n");
        if (file && info.frames > 0) {

            float binSize = (float)info.frames / lms_graph_width;
            float peak[2] = { 0.0f, 0.0f }, mean[2] = { 0.0f, 0.0f };
            float *frame = (float *)malloc(info.channels * sizeof(float));
            int bin = 0;

            QPainter paint(&(lms_graphs[a_index]));

            for (size_t i = 0; i < info.frames; ++i) {

                sf_readf_float(file, frame, 1);

                if (fabs(frame[0]) > peak[0]) peak[0] = fabs(frame[0]);
                mean[0] += fabs(frame[0]);

                if (info.channels > 1) {
                    if (fabs(frame[1]) > peak[1]) peak[1] = fabs(frame[1]);
                    mean[1] += fabs(frame[1]);
                }

                if (i == size_t((bin + 1) * binSize)) {

                    float silent = 1.0 / float(lms_graph_height);

                    if (info.channels == 1) {
                        mean[1] = mean[0];
                        peak[1] = peak[0];
                    }

                    mean[0] /= binSize;
                    mean[1] /= binSize;

                    int m = lms_graph_height / 2;

                    paint.setPen(Qt::black);
                    paint.drawLine(bin, m, bin, int(m - m * peak[0]));
                    if (peak[0] > silent && peak[1] > silent) {
                        paint.drawLine(bin, m, bin, int(m + m * peak[1]));
                    }

                    paint.setPen(Qt::gray);
                    paint.drawLine(bin, m, bin, int(m - m * mean[0]));
                    if (mean[0] > silent && mean[1] > silent) {
                        paint.drawLine(bin, m, bin, int(m + m * mean[1]));
                    }

                    paint.setPen(Qt::black);
                    paint.drawPoint(bin, int(m - m * peak[0]));
                    if (peak[0] > silent && peak[1] > silent) {
                        paint.drawPoint(bin, int(m + m * peak[1]));
                    }

                    mean[0] = mean[1] = 0.0f;
                    peak[0] = peak[1] = 0.0f;

                    ++bin;
                }
            }

            lms_durations[a_index] = int(100.0 * float(info.frames) / float(info.samplerate));

            /*
            m_sample_counts[a_index] = info.frames;
            m_sample_start_fine->setMaximum(info.frames);
            m_sample_end_fine->setMaximum(info.frames);
            m_loop_start_fine->setMaximum(info.frames);
            m_loop_end_fine->setMaximum(info.frames);

            //Set seconds
            QTableWidgetItem *f_set_seconds = new QTableWidgetItem;
            QString * f_seconds = new QString();                
            f_seconds->setNum((float(info.frames) / float(info.samplerate)));
            f_set_seconds->setText(*f_seconds);
            m_sample_table->setItem(a_index, 11, f_set_seconds);

            //Set samples
            QTableWidgetItem *f_set_samples = new QTableWidgetItem;
            QString * f_samples = new QString();                
            f_samples->setNum((info.frames));
            f_set_samples->setText(*f_samples);
            m_sample_table->setItem(a_index, 12, f_set_samples);

            //Trigger start/end changes to update m_sample_table
            sampleStartChanged(m_sample_start->value());
            sampleEndChanged(m_sample_start->value());
            loopStartChanged(m_sample_start->value());
            loopEndChanged(m_sample_start->value());
            */

        } 
        
        if (file) sf_close(file);
        
        //m_sample_graph->setPixmap(lms_graphs[a_index]);
    }

    
};

#endif	/* SAMPLE_GRAPH_H */

